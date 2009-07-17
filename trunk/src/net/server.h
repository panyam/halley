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
 *  \class  server.h
 *
 *  \brief  Superclass of all multi-threaded servers.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SSERVER_H_
#define _SSERVER_H_

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
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <list>
#include <string>
#include <vector>

#include "thread/task.h"

//*****************************************************************************
/*!
 *  \class  SServer
 *
 *  \brief Superclass of all servers.
 *
 *****************************************************************************/
class SServer : public STask
{
public:
    //! Constructor
    SServer(int port_ = -1) : serverPort(port_) { }

    //! Destructor
    ~SServer();

    //! Return the server's port
    inline int GetPort() { return this->serverPort; }

    //! Set the server's port
    inline void SetPort(int port) { this->serverPort = port; }

protected:
    // Creates the socket
    // virtual int CreateSocket();

    // Called to stop the task.
    virtual int RealStop();

    // Main task method
    virtual int Run();

    // Creates a server socket 
    virtual int CreateSocket();

    // Binds the socket
    virtual int BindSocket();

    // Listens on the socket
    virtual int ListenOnSocket();

    // handle a new connection
    virtual void HandleConnection(int clientSocket);

private:
    //! Declared functions but not implemented.
    SServer(const SServer &);
    SServer & operator=(const SServer &);

private:
    //! The server port
    unsigned            serverPort;

    //! The server socket
    int                 serverSocket;

    //! To indicate whether the server has been stopped
    bool                serverStopped;
};

#endif

