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
#define MAXEPOLLTIME    50

//*****************************************************************************
/*!
 *  \brief  Creates a new server.
 *
 *  \version
 *      - Sri Panyam      16/04/2009
 *        Created.
 *
 *****************************************************************************/
SEvServer::SEvServer(int port_, SReaderStage* pReaderStage_, SWriterStage *pWriterStage_) :
    serverPort(port_),
    serverSocket(-1),
    serverEpollFD(-1),
    pReaderStage(pReaderStage_),
    pWriterStage(pWriterStage_)
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
    if (SEDSUtils::SetNonBlocking(clientSocket))
    {
        SLogger::Get()->Log("ERROR: Cannot make client socket non blocking[%d]: %s\n\n", errno, strerror(errno));
        return -1;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt SO_REUSEADDR failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const void *)&nodelay, sizeof(nodelay)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt TCP_NODELAY failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    /*
    int keepalive = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, (const void *)&keepalive, sizeof(keepalive)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt SO_KEEPALIVE failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }
    */


#ifndef DISABLE_SO_LINGER
    struct linger linger;
    linger.l_onoff = 1;
    linger.l_linger = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, (const void *)&linger, sizeof(struct linger)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt SO_LINGER failed: [%d]: %s\n\n", errno, strerror(errno));
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
        SLogger::Get()->Log("ERROR: Cannot create server socket: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    if (SEDSUtils::SetNonBlocking(newSocket))
    {
        SLogger::Get()->Log("ERROR: setnonblocking failed: [%d]: %s\n\n", errno, strerror(errno));
        return -1;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt (SO_REUSEADDR) failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(newSocket, IPPROTO_TCP, TCP_NODELAY, (const void *)&nodelay, sizeof(nodelay)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt (TCP_NODELAY) failed: [%d]: %s\n\n", errno, strerror(errno));
        return -errno;
    }

    int timeout = 0;
    if (setsockopt(newSocket, IPPROTO_TCP, TCP_DEFER_ACCEPT, (const void *)&timeout, sizeof(timeout)) != 0)
    {
        SLogger::Get()->Log("ERROR: setsockopt (TCP_DEFER_ACCEPT) failed: [%d]: %s\n\n", errno, strerror(errno));
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
        SLogger::Get()->Log("ERROR: bind failed: [%d]: %s\n\n", errno, strerror(errno));
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
    int retval = listen(serverSocket, SOMAXCONN);
    if (retval != 0)
    {
        SLogger::Get()->Log("ERROR: listen failed: [%d]: %s\n\n", errno, strerror(errno));
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
        SLogger::Get()->Log("ERROR: setrlimit failed: [%d]: %s\n\n", errno, strerror(errno));
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

    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];

    bzero(&ev, sizeof(ev));
    ev.events   = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR;
    ev.data.ptr = NULL;
    if (epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, serverSocket, &ev) < 0)
    {
        SLogger::Get()->Log("ERROR: epoll_ctl failed: [%d]: %s\n\n", errno, strerror(errno));
        close(serverEpollFD);
        serverEpollFD = -1;
        return errno;
    }

    // now run the server asynchronously
    while (!Stopped())
    {
        int nfds = epoll_wait(serverEpollFD, events, MAXEPOLLSIZE, MAXEPOLLTIME);

        if (nfds < 0)
        {
            if (errno != EINTR)
            {
                SLogger::Get()->Log("ERROR: epoll_wait failed: [%d]: %s\n\n", errno, strerror(errno));
                break ;
            }
        }
        else
        {
            // go through and remove connections that are
            // marked as close if their reference counts are 0
            CloseMarkedConnections();
        }

        for (int n = 0;!Stopped() && n < nfds;n++)
        {
            SConnection *   pConnection = (SConnection *)(events[n].data.ptr);
            int             connSocket  = pConnection == NULL ? serverSocket : pConnection->Socket();
            int             event_flags = events[n].events;

            if ((event_flags & (EPOLLERR | EPOLLHUP)) &&
                    (event_flags & (EPOLLIN | EPOLLOUT)) == 0)
            {
                // if error events were available without input or output
                // events then add these in too
                SLogger::Get()->Log("ERROR: Found Error event - Setting EPOLLIN and EPOLLOUT: \n\n");
                event_flags |= (EPOLLIN | EPOLLOUT);

                // we remove this socket from the epoll list but do
                // not kill this as it will be done from a
                // different point (reader stage) anyway
                if (pConnection != NULL)
                {
                    SLogger::Get()->Log("TRACE: Hangup Recieved - Connection: [%x], Socket: [%d]\n", pConnection, pConnection->Socket());
                    MarkConnectionAsClosed(pConnection);
                }
            }

            if ((event_flags & EPOLLIN) != 0)
            {
                if (connSocket == serverSocket)    // its a connection request
                {
                    sockaddr_in client_sock_addr;
                    socklen_t addlen = sizeof(client_sock_addr);

                    int clientSocket = 0;
                    
                    while (!Stopped() && clientSocket >= 0)
                    {
                        clientSocket = accept(serverSocket, (struct sockaddr *)&client_sock_addr, &addlen);
                        if (clientSocket < 0)
                        {
                            if (errno != EAGAIN)
                            {
                                SLogger::Get()->Log("ERROR: accept failed: [%d]: %s\n\n", errno, strerror(errno));
                                assert("Accept Failed" && false);
                            }
                        }
                        else if (Stopped())
                        {
                            int result = close(clientSocket);
                            if (result != 0)
                            {
                                SLogger::Get()->Log("ERROR: clientSocket close failed: [%d]: %s\n\n", errno, strerror(errno));
                            }
                            break ;
                        }
                        else
                        {
                            // Set additional options on the client socket
                            // including non-blocking
                            PrepareClientSocket(clientSocket);

                            // creates a new connection
                            NewConnection(clientSocket);
                        }
                    }
                }
                else    // on a IO by a connection
                {
                    // means we have data to read off this socket, 
                    // dont read it but give it the request reader task
                    // handler!
                    pReaderStage->SendEvent_ReadRequest(pConnection);
                }
            }

            if ((event_flags & EPOLLOUT) != 0)
            {
                SLogger::Get()->Log("TRACE: PollOut For Connection: [%x], Socket: [%d], State: [%d]:\n", pConnection, pConnection->Socket(), pConnection->GetState());
                // we are writing out to a socket
                pWriterStage->SendEvent_WriteData(pConnection);
            }
        }
    }

    // close connections marked as close
    CloseMarkedConnections();

    // Close all client sockets.  Note we could do all this in
    // RealStop, but the problem is that RealStop is (usually) called from 
    // a different thread which means while we are closing these sockets 
    // there could be action on the main server thread which we dont want.
    while ( ! connections.empty())
    {
        std::set<SConnection *>::iterator   iter        = connections.begin();
        SConnection *                       pConnection = *iter;
        connections.erase(pConnection);
        delete pConnection;
    }

    if (serverSocket >= 0)
    {
        // and finally the server socket
        // shutdown(serverSocket, SHUT_RDWR);
        int result = close(serverSocket);
        if (result != 0)
        {
            SLogger::Get()->Log("ERROR: serverSocket close failed: [%d]: %s\n\n", errno, strerror(errno));
        }
        serverSocket = -1;
    }

    if (serverEpollFD >= 0)
    {
        int result = close(serverEpollFD);
        if (result != 0)
        {
            SLogger::Get()->Log("ERROR: serverEpollFD close failed: [%d]: %s\n\n", errno, strerror(errno));
        }
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
*   \brief  Notified by the reader when the peer has closed the connection.
*
*   \version
*       - Sri Panyam  14/07/2009
*         Created
**************************************************************************************/
void SEvServer::PeerClosedConnection(SConnection *pConnection)
{
    if (pConnection == NULL) return ;

    SMutexLock locker(connListMutex);

    SLogger::Get()->Log("TRACE: Peer Closed Connection: [%x], Socket: [%d]\n", pConnection, pConnection->Socket());

    struct epoll_event ev;
    bzero(&ev, sizeof(ev));

    // Remove the read-events since the peer has closed the connection
    ev.events       = EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR;
    ev.data.ptr     = pConnection;

    if (epoll_ctl(serverEpollFD, EPOLL_CTL_MOD, pConnection->Socket(), &ev) < 0)
    {
        SLogger::Get()->Log("ERROR: epoll_ctl MOD error [%d]: %s\n", errno, strerror(errno));
        assert("Could not remove POLLIN from socket events" && false);
    }
}

/**************************************************************************************
*   \brief  Marks a connection as closed so when time comes it can be
*   deleted.
*
*   \version
*       - Sri Panyam  09/07/2009
*         Created
**************************************************************************************/
void SEvServer::MarkConnectionAsClosed(SConnection *pConnection)
{
    if (pConnection == NULL) return ;

    SMutexLock locker(connListMutex);

    SLogger::Get()->Log("TRACE: Marking As Close - Connection: [%x], Socket: [%d]\n", pConnection, pConnection->Socket());
    if (connections.find(pConnection) != connections.end())
    {
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));

        if (epoll_ctl(serverEpollFD, EPOLL_CTL_DEL, pConnection->Socket(), &ev) < 0)
        {
            SLogger::Get()->Log("ERROR: epoll_ctl delete error [%d]: %s\n", errno, strerror(errno));
        }

        // move the connection to the closed connection list
        pConnection->SetState(SConnection::STATE_CLOSED);
        connections.erase(pConnection);

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
    SMutexLock locker(connListMutex);
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

/**************************************************************************************
*   \brief  Creates a new connection and adds to the list.
*
*   \version
*       - Sri Panyam  09/07/2009
*         Created
**************************************************************************************/
SConnection *SEvServer::NewConnection(int clientSocket)
{
    SConnection *pConn = new SConnection(this, clientSocket);

    assert("Could not create new connection" && pConn != NULL);

    if (pConn != NULL)
    {
        SMutexLock locker(connListMutex);
        connections.insert(pConn);

        // what about EPOLLOUT??
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.events       = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR;
        ev.data.ptr     = pConn;

        if (epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, clientSocket, &ev) < 0)
        {
            SLogger::Get()->Log("ERROR: epoll_ctl error [%d]: %s\n", errno, strerror(errno));
            assert("Could not add connection to epoll list" && false);
        }
    }

    return pConn;
}

