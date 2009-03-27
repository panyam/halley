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
 *  \file   connhandler.h
 *
 *  \brief  Handlers for each connection in a server.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SCONNHANDLER_H_
#define _SCONNHANDLER_H_

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

#include <iostream>
#include <fstream>

#include <list>
#include <string>

#include "thread/task.h"

// Forward declare this class
class SServer;
class SConnHandler;

//*****************************************************************************
/*!
 *  \class  SConnPoolListener
 *
 *  \brief  A connection handler listener.
 *
 *****************************************************************************/
class SConnPoolListener 
{
public:
    //! Virtual destructor
    virtual ~SConnPoolListener() { }

    //! Called when handler is created
    virtual void HandlerCreated(SConnHandler *pHandler) { }

    //! Called just before handler is destroyed
    virtual void HandlerDestroyed(SConnHandler *pHandler) { }
};

//*****************************************************************************
/*!
 *  \class  SConnHandler
 *
 *  \brief  Objects that handle the connections for the server.
 *
 *****************************************************************************/
class SConnHandler : public STask
{
public:
    SConnHandler();
    virtual ~SConnHandler();

    // Initialises the client
    virtual void Init(SServer *parent, int clientSocket);
    virtual void Reset();
    virtual int  Socket() { return clientSocket; }

    //! Handles a new connection
    // Returns true to indicate connection is done handling.
    virtual bool    HandleConnection() { Start(); return true; }

protected:
    //! The real stop function
    virtual int     RealStop();

protected:
    SServer *           pServer;
    // The client socket
    int                 clientSocket;

    // socket buffer
    std::istream *      clientInput;
    std::ostream *      clientOutput;
    bool                clientStopped;
};

#endif

