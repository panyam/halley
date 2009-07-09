//*****************************************************************************
/*!
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 *
 *  \file   evserver.cpp
 *
 *  \brief
 *  An event driven server.
 *
 *  \version
 *      - Sri Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#include <string.h>

#include "server.h"
#include "connection.h"
#include "writerstage.h"
#include "readerstage.h"

#define MAXEPOLLSIZE    10000

#ifndef EPOLLRDHUP
#define EPOLLRDHUP  0x2000
#endif

//*****************************************************************************
/*!
 *  \brief  Creates a new server.
 *
 *  \version
 *      - Sri Panyam      16/04/2009
 *        Created.
 *
 *****************************************************************************/
SEvServer::SEvServer(int port_, SReaderStage* pReaderStage_) :
    serverPort(port_),
    serverSocket(-1),
    serverEpollFD(-1),
    pReaderStage(pReaderStage_)
{
}

//*****************************************************************************
/*!
 *  \brief  Called to stop the server.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
int
SEvServer::RealStop()
{
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Prepares a client socket by setting options specific to the
 *  application.
 *
 *  \version
 *      - Sri Panyam      07/07/2009
 *        Created.
 *
 *****************************************************************************/
int SEvServer::PrepareClientSocket(int clientSocket)
{
    if (setnonblocking(clientSocket))
    {
        SLogger::Get()->Log(0, "ERROR: Cannot make client socket non blocking[%d]: %s\n\n", errno, strerror(errno));
        return -1;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt SO_REUSEADDR failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const void *)&nodelay, sizeof(nodelay)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt TCP_NODELAY failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }


#ifndef DISABLE_SO_LINGER
    struct linger linger;
    linger.l_onoff = 1;
    linger.l_linger = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, (const void *)&linger, sizeof(struct linger)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt SO_LINGER failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }
#endif

    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Creates the server socket.
 *
 *  Here we can allow child classes to do custom socket creations
 *
 *  \version
 *      - Sri Panyam      16/02/2009
 *        Created.
 *
 *****************************************************************************/
int SEvServer::CreateSocket()
{
    // Create an internet socket using streaming (tcp/ip)
    // and save the handle for the server socket
    int newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket < 0)
    {
        SLogger::Get()->Log(0, "ERROR: Cannot create server socket: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    if (setnonblocking(newSocket))
    {
        SLogger::Get()->Log(0, "ERROR: setnonblocking failed: [%d]: %s\n\n", errno, strerror(errno));
        return -1;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt (SO_REUSEADDR) failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(newSocket, IPPROTO_TCP, TCP_NODELAY, (const void *)&nodelay, sizeof(nodelay)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt (TCP_NODELAY) failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int timeout = 0;
    if (setsockopt(newSocket, IPPROTO_TCP, TCP_DEFER_ACCEPT, (const void *)&timeout, sizeof(timeout)) != 0)
    {
        SLogger::Get()->Log(0, "ERROR: setsockopt (TCP_DEFER_ACCEPT) failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    return newSocket;
}

//*****************************************************************************
/*!
 *  \brief  Bind the socket to the host and port.
 *
 *  \version
 *      - Sri Panyam      16/02/2009
 *        Created.
 *
 *****************************************************************************/
int SEvServer::BindSocket()
{
    // Setup the structure that defines the IP-adress, port and protocol
    // family to use.
    sockaddr_in srv_sock_addr;

    // zero the srv sock addr structure
    bzero(&srv_sock_addr, sizeof(srv_sock_addr));

    // Use the internet domain.
    srv_sock_addr.sin_family = AF_INET;

    // Use this specific port.
    srv_sock_addr.sin_port = htons(serverPort);

    // Use any of the network cards IP addresses.
    srv_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the port number specified by (serverPort).
    int retval = bind(serverSocket, (sockaddr*)(&srv_sock_addr), sizeof(sockaddr));
    if (retval != 0)
    {
        SLogger::Get()->Log(0, "ERROR: bind failed: [%d]: %s\n\n", errno, strerror(errno));
        return errno;
    }
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Starts listening on the socket for connections.
 *
 *  \version
 *      - Sri Panyam      16/02/2009
 *        Created.
 *
 *****************************************************************************/
int SEvServer::ListenOnSocket()
{
    // Setup a limit of maximum 10 pending connections.
    int retval = listen(serverSocket, 10);
    if (retval != 0)
    {
        SLogger::Get()->Log(0, "ERROR: listen failed: [%d]: %s\n\n", errno, strerror(errno));
        return errno;
    }
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Runs the server thread.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SEvServer::Run()
{
    if ((serverSocket = CreateSocket()) < 0)
    {
        return serverSocket;
    }

    int result = BindSocket();
    if (result != 0) return result;

    result = ListenOnSocket();
    if (result != 0) return result;

    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
    // A few issues with setrlimit and valgrind so disable this in
    // this mode
#ifndef USING_VALGRIND
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
    {
        SLogger::Get()->Log(0, "ERROR: setrlimit failed: [%d]: %s\n\n", errno, strerror(errno));
        return errno;
    }
#endif

    if (serverEpollFD >= 0)
    {
        close(serverEpollFD);
        serverEpollFD = -1;
    }

    // Ignore SIGPIPE' so we can ignore error's 
    // thrown by out of band data
    signal(SIGPIPE, SIG_IGN);

    serverEpollFD       = epoll_create(MAXEPOLLSIZE);
    int         curfds  = 1;
    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];

    bzero(&ev, sizeof(ev));
    ev.events   = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP;
    ev.data.ptr = NULL;
    if (epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, serverSocket, &ev) < 0)
    {
        SLogger::Get()->Log(0, "ERROR: epoll_ctl failed: [%d]: %s\n\n", errno, strerror(errno));
        close(serverEpollFD);
        serverEpollFD = -1;
        return errno;
    }

    // now run the server asynchronously
    while (!Stopped())
    {
        int nfds = epoll_wait(serverEpollFD, events, curfds, 1000);

        if (nfds != 0)
        {
            if (nfds < 0 && errno != EINTR)
            {
                SLogger::Get()->Log(0, "ERROR: epoll_wait failed: [%d]: %s\n\n", errno, strerror(errno));
                break ;
            }

            if (errno != EINTR)
            {
                for (int n = 0;!Stopped() && n < nfds;n++)
                {
                    SConnection *   pConnection = (SConnection *)(events[n].data.ptr);
                    int connSocket  = pConnection == NULL ? serverSocket : pConnection->Socket();
                    if (connSocket == serverSocket)    // its a connection request
                    {
                        sockaddr_in client_sock_addr;
                        socklen_t addlen = sizeof(client_sock_addr);

                        // go through and remove connections that are
                        // marked as close if their reference counts are 0
                        CloseMarkedConnections();

                        int clientSocket = accept(serverSocket, (struct sockaddr *)&client_sock_addr, &addlen);

                        if (clientSocket < 0)
                        {
                            SLogger::Get()->Log(0, "ERROR: accept failed: [%d]: %s\n\n", errno, strerror(errno));
                            break ;
                        }
                        else if (Stopped())
                        {
                            // server being killed and we have a socket, 
                            // so close the socket as well
                            int result = close(clientSocket);
                            if (result != 0)
                            {
                                SLogger::Get()->Log(0, "ERROR: close failed: [%d]: %s\n\n", errno, strerror(errno));
                            }
                        }
                        else
                        {
                            // Set additional options on the client socket
                            // including non-blocking
                            PrepareClientSocket(clientSocket);

                            // should we use 
                            SConnection *pConn  = new SConnection(this, clientSocket);
                            connections.insert(pConn);

                            // what about EPOLLOUT??
                            ev.events   = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP;
                            ev.data.ptr         = pConn;

                            if (epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, clientSocket, &ev) < 0)
                            {
                                SLogger::Get()->Log(0, "ERROR: epoll_ctl error [%d]: %s\n", errno, strerror(errno));
                                break ;
                            }
                            else
                            {
                                curfds++;
                            }
                        }
                    }
                    else if ((events[n].events & EPOLLOUT) != 0)
                    {
                        // we are writing out to a socket
                        pWriterStage->SendEvent_WriteData(pConnection);
                    }
                    else if ((events[n].events & EPOLLIN) != 0)
                    {
                        // means we have data to read off this socket, 
                        // dont read it but give it the request reader task
                        // handler!
                        pReaderStage->SendEvent_ReadRequest(pConnection);
                    }
                    else if ((events[n].events & (EPOLLRDHUP | EPOLLHUP)) != 0)
                    {
                        // we remove this socket from the epoll list but do
                        // not kill this as it will be done from a
                        // different point (reader stage) anyway
                        if (pConnection != NULL)
                        {
                            events[n].data.ptr = NULL;
                            // move the connection to the closed connection list
                            pConnection->SetState(SConnection::STATE_CLOSED);
                            if (epoll_ctl(serverEpollFD, EPOLL_CTL_DEL, pConnection->Socket(), &ev) < 0)
                            {
                                SLogger::Get()->Log(0, "ERROR: epoll_ctl delete error [%d]: %s\n", errno, strerror(errno));
                            }
                            else
                            {
                                curfds--;
                            }

                            connections.erase(pConnection);
                            pConnection->CloseSocket();

                            // move the connection to the closed connections list
                            // so it can be freed up when we need spare ones
                            if (pConnection->RefCount() == 0)
                            {
                                delete pConnection;
                            }
                            else
                            {
                                closedConnections.push_back(pConnection);
                            }
                        }
                    }
                }
            }
        }
    }

    // Close all server and client sockets.  Note we could do all this in
    // RealStop, but the problem is that RealStop is (usually) called from 
    // a different thread which means while we are closing these sockets 
    // there could be action on the main server thread which we dont want.
    while ( ! connections.empty())
    {
        std::set<SConnection *>::iterator iter = connections.begin();
        ConnectionComplete(*iter);
    }

    if (serverSocket >= 0)
    {
        // and finally the server socket
        // shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }

    if (serverEpollFD >= 0)
    {
        close(serverEpollFD);
        serverEpollFD = -1;
    }
    return result;
}

/**************************************************************************************
*   \brief  Destructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SEvServer::~SEvServer()
{
    Stop();

    if (serverSocket >= 0)
    {
        close(serverSocket);
        serverSocket = -1;
    }

    if (serverEpollFD >= 0)
    {
        close(serverEpollFD);
        serverEpollFD = -1;
    }
}

/**************************************************************************************
*   \brief  Sets a socket as non blocking.
*
*   \version
*       - Sri Panyam  18/02/2009
*         Created
**************************************************************************************/
int SEvServer::setnonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) == -1)
        return -1;
    return 0;
}

/**************************************************************************************
*   \brief  Sets an event stage by a given name.
*
*   \version
*       - Sri Panyam  19/02/2009
*         Created
**************************************************************************************/
void SEvServer::SetStage(const SString &name, SStage *pStage)
{
    std::map<SString, SStage *>::iterator iter = eventStages.find(name);
    if (iter == eventStages.end())
    {
        eventStages.insert(std::pair<SString, SStage *>(name, pStage));
    }
    else
    {
        eventStages[name] = pStage;
        eventStages.insert(std::pair<SString, SStage *>(name, pStage));
    }
}

/**************************************************************************************
*   \brief  Gets an event stage by a given name.
*
*   \version
*       - Sri Panyam  19/02/2009
*         Created
**************************************************************************************/
SStage *SEvServer::GetStage(const SString &name)
{
    std::map<SString, SStage *>::iterator iter = eventStages.find(name);
    if (iter == eventStages.end())
    {
        return NULL;
    }
    else
    {
        return iter->second;
    }
}

/**************************************************************************************
*   \brief  Called by a connection when it is done with.
*
*   \version
*       - Sri Panyam  19/02/2009
*         Created
**************************************************************************************/
void SEvServer::ConnectionComplete(SConnection *pConnection)
{
    if (connections.end() != connections.find(pConnection))
    {
        connections.erase(pConnection);
        delete pConnection;
    }
}

/**************************************************************************************
*   \brief  Destroys connections marked as closed if their refcounts have
*   reached 0.
*
*   \version
*       - Sri Panyam  08/07/2009
*         Created
**************************************************************************************/
void SEvServer::CloseMarkedConnections()
{
    std::list<SConnection *>::iterator iter = closedConnections.begin();
    while (iter != closedConnections.end())
    {
        SConnection *pConnection = *iter;
        if (pConnection->RefCount() == 0)
        {
            iter = closedConnections.erase(iter);
            delete pConnection;
        }
        else
        {
            ++iter;
        }
    }
}

