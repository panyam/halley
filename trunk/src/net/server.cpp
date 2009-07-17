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
 *  \file   server.cpp
 *
 *  \brief  Superclass of all multithreaded servers.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "net/server.h"
#include "net/connhandler.h"
#include "net/connpool.h"
#include "net/sockbuff.h"

#include <string.h>

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
    serverStopped = true;

    if (serverSocket >= 0)
    {
        close(serverSocket);
        serverSocket = -1;
    }

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

        cerr << "Accepted new connection on socket: " << serverSocket << endl;

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
    // do nothing - override to do task specific stuff
    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
    cerr << "Done Handling Connection: " << clientSocket << endl;
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

