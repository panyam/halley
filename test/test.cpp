
#include <sys/epoll.h>
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <netdb.h>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>

#include <list>
#include <string>
#include <vector>

using namespace std;

#define MAXBUF          8
#define MAXEPOLLSIZE    10000

class EvData
{
public:
    EvData(int sock) : ourSocket(sock) { }
    int ourSocket;
};

int setnonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) == -1)
        return -1;
    return 0;
}

int handle_message(void *ptr)
{
    EvData *evData = (EvData *)ptr;
    char buf[MAXBUF + 1];
    int len;
    bzero(buf, MAXBUF + 1);
    len = recv(evData->ourSocket, buf, MAXBUF, 0);
    if (len > 0)
    {
        printf("New Message (Ptr: %p, Ptr->Socket: %d, size: %d): %s", evData, evData->ourSocket, len, buf);
    }
    else if (len < 0)
    {
        cerr << "ERROR: recv error: [" << errno << "]: " << strerror(errno) << endl << endl;
        return -errno;
    }
    return len;
}

int main(int argc, char *argv[])
{
    socklen_t len;
    struct sockaddr_in their_addr;
    unsigned int myport = argc <= 1 ? 9090 : atoi(argv[1]),
                 lisnum = argc <= 2 ? 2 : atoi(argv[2]);

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        cerr << "ERROR: Cannot create server socket: [" << errno << "]: " << strerror(errno) << endl << endl;
        return -errno;
    }

    setnonblocking(listener);

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
    {
        cerr << "ERROR: setsockopt failed: [" << errno << "]: " 
             << strerror(errno) << endl << endl;
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << endl << endl;
        return -errno;
    }

    // Setup the structure that defines the IP-adress, port and protocol
    // family to use.
    sockaddr_in srv_sock_addr;

    // zero the srv sock addr structure
    bzero(&srv_sock_addr, sizeof(srv_sock_addr));

    // Use the internet domain.
    srv_sock_addr.sin_family = AF_INET;

    // Use this specific port.
    srv_sock_addr.sin_port = htons(myport);

    // Use any of the network cards IP addresses.
    srv_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the port number specified by (myport).
    int retval = bind(listener, (sockaddr*)(&srv_sock_addr), sizeof(sockaddr));
    if (retval != 0)
    {
        cerr << "ERROR: Cannot bind to server on port: [" << errno << "]: " 
             << strerror(errno) << endl;
        cerr << "ERROR: --------------------------------------------------" << endl;
        return errno;
    }

    // Setup a limit of maximum lisnum pending connections.
    retval = listen(listener, lisnum);
    if (retval != 0)
    {
        cerr << "ERROR: Cannot listen to connections: [" << errno << "]: " 
             << strerror(errno) << endl;
        return errno;
    }

    // create epoll handle and add the socket in epoll 
    int new_fd, kdpfd, nfds, n, ret, curfds;
    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];
    struct rlimit rt;

    printf("String Size: %d\n", sizeof(std::string));
    printf("Epoll Evt Size: %d\n", sizeof(epoll_event));

    rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;

    // what does this do?
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
    {
        perror("setrlimit");
        exit(1);
    }
    else
    {
        printf("Successful parameter setting of system resources!\n");
    }
    kdpfd       = epoll_create(MAXEPOLLSIZE);
    len         = sizeof(struct sockaddr_in);
    ev.events   = EPOLLIN | EPOLLET;
    ev.data.ptr = new EvData(listener);
    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listener, &ev) < 0)
    {
        cerr << "ERROR: epoll_ctl error: [" << errno << "]: " << strerror(errno) << endl;
        return errno;
    }

    curfds = 1;
    while (true)
    {
        nfds = epoll_wait(kdpfd, events, curfds, -1);
        if (nfds == -1)
        {
            cerr << "ERROR: epoll_wait error: [" << errno << "]: " << strerror(errno) << endl;
            return errno;
        }

        for (n = 0;n <nfds;n++)
        {
            if (((EvData *)events[n].data.ptr)->ourSocket == listener)
            {
                new_fd = accept(listener, (struct sockaddr *)&their_addr, &len);
                if (new_fd < 0)
                {
                    cerr << "ERROR: accept error: [" << errno << "]: " << strerror(errno) << endl;
                    return errno;
                }

                setnonblocking(new_fd);
                ev.events = EPOLLIN; // | EPOLLET;
                ev.data.ptr = new EvData(new_fd);

                if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, new_fd, &ev) < 0)
                {
                    cerr << "ERROR: epoll_ctl(new_fd) error: [" << errno << "]: " << strerror(errno) << endl;
                    return errno;
                }
                curfds++;
            }
            else
            {
                ret = handle_message(events[n].data.ptr);
                if (ret < 1 && errno != 11)
                {
                    epoll_ctl(kdpfd, EPOLL_CTL_DEL, ((EvData *)events[n].data.ptr)->ourSocket, &ev);
                    curfds--;
                }
            }
        }
    }
    close(listener);

    return 0;
}

