//*****************************************************************************
/*!
 *  \file   tcpserver.cpp
 *
 *  \brief
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "net/server.h"
#include "net/connhandler.h"
#include "net/connpool.h"
#include "net/sockbuff.h"

#include <string.h>
#include <event.h>

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
SServer::RealStop()
{
    std::cerr << "INFO: Stopping Server..." << endl;

    serverStopped = true;

    if (serverSocket >= 0)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }

    // Stop child threads first
    SMutexLock    mutexLock(handlerListMutex);
    for (std::list<SConnHandler *>::iterator iter=connHandlers.begin();
            iter != connHandlers.end();
            ++iter)
    {
        // stop the client first
        if ((*iter)->Running())
        {
            (*iter)->Stop();
        }

        // then the client
        delete (*iter);
    }

    // clear client list
    connHandlers.clear();

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
int SServer::CreateSocket()
{
    // Create an internet socket using streaming (tcp/ip)
    // and save the handle for the server socket
    int newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket < 0)
    {
        cerr << "ERROR: Cannot create server socket: [" << errno << "]: " << strerror(errno) << endl << endl;
        return -errno;
    }

    if (setnonblocking(newSocket))
    {
        cerr << "ERROR: Cannot make socket non blocking: [" << errno << "]: " << strerror(errno) << endl << endl;
        return -1;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
    {
        cerr << "ERROR: setsockopt failed: [" << errno << "]: " 
             << strerror(errno) << endl << endl;
        return -errno;
    }

    int nodelay = 1;
    if (setsockopt(newSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << endl << endl;
        return -errno;
    }

    if (setsockopt(newSocket, SOL_SOCKET, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << endl << endl;
        return -errno;
    }

    if (setsockopt(newSocket, SOL_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << endl << endl;
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
int SServer::BindSocket()
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
        cerr << "ERROR: Cannot bind to server on port: [" << errno << "]: " 
             << strerror(errno) << endl;
        cerr << "ERROR: --------------------------------------------------" << endl;
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
int SServer::ListenOnSocket()
{
    // Setup a limit of maximum 10 pending connections.
    int retval = listen(serverSocket, 10);
    if (retval != 0)
    {
        cerr << "ERROR: Cannot listen to connections: [" << errno << "]: " 
             << strerror(errno) << endl;
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
int SServer::Run()
{
    if ((serverSocket = CreateSocket()) < 0)
    {
        return serverSocket;
    }

    int result = BindSocket();
    if (result != 0) return result;

    result = ListenOnSocket();
    if (result != 0) return result;

    // now start accepting those connection thingies.
    serverStopped = false;

    // Ignore SIGPIPE' so we can ignore error's 
    // thrown by out of band data
    signal(SIGPIPE, SIG_IGN);

    while(!serverStopped)
    {
        // Accept will block until a request is detected on this socket - port 80 - HTTP
        sockaddr_in client_sock_addr;
        socklen_t addlen = sizeof(client_sock_addr);

        // Handle for the socket taking the client request
        // TODO: Limit max connections to avoid flooding - 
        // but this is an inhouse thing
        int clientSocket = accept(serverSocket, (sockaddr*)(&client_sock_addr), &addlen);
        int errnum  = errno;
        result      = errnum;

        if (!serverStopped)
        {
            if (clientSocket < 0)
            {
                cerr << "ERROR: Could not accept connection [" << errnum << "]: " << strerror(errnum) << "." << endl;
                serverStopped = true;
            }
            else
            {
                HandleConnection(clientSocket);
            }
        }
        else if (clientSocket >= 0)
        {
            // server being killed and we have a socket, so close the
            // socket as well
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
        }
    }

    if (serverSocket >= 0)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }

    return result;
}

//*****************************************************************************
/*!
 *  \brief  Handles a connection request by a client.
 *
 *  \param  int clientSocket    The client socket being handled.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SServer::HandleConnection(int clientSocket)
{
    bool handled = false;
    std::cerr << "Handling Connection: " << clientSocket << std::endl;
    if (connFactory != NULL)
    {
        SConnHandler *handler = connFactory->NewHandler();
        if (handler != NULL)
        {
            handler->Init(this, clientSocket);
            handled = handler->HandleConnection();
            if (!handled)
            {
                // means handler is in asynchronous mode - so add it to our
                // list so clients, so the handler will have to explicitly
                // call back when it is done with
                SMutexLock locker(handlerListMutex);

                // TODO: assert if handler is already in the handler list!
                connHandlers.push_back(handler);
            }
            else
            {
                connFactory->ReleaseHandler(handler);
            }
        }
    }

    if (handled)
    {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        cerr << "Done Handling Connection: " << clientSocket << endl;
    }
}

/**************************************************************************************
*   \brief  Destructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SServer::~SServer()
{
    Stop();
}

/**************************************************************************************
*   \brief  Sets a socket as non blocking.
*
*   \version
*       - Sri Panyam  18/02/2009
*         Created
**************************************************************************************/
int SServer::setnonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) == -1)
        return -1;
    return 0;
}

