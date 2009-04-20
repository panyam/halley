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
 *  \file   connection.h
 *
 *  \brief
 *
 *  A connection in an event driver server.
 *
 *  \version
 *      - Sri Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SCONNECTION_H_
#define _SCONNECTION_H_

#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <iomanip>
#include <iostream>

#include "net/sockbuff.h"
#include "eds/job.h"

//*****************************************************************************
/*!
 *  \class  SConnection
 *
 *  \brief A connection recieved by our server.
 *
 *****************************************************************************/
class SConnection : public SJob
{
public:
    //! Creates a new connection
    SConnection(SEvServer *pSrv, int sock);

    //! Destroys the connection object
    virtual ~SConnection();

    //! Get the socket associated with the connection
    int Socket() { return connSocket; }

    //! Get the socket associated with the connection
    SEvServer *Server() { return pServer; }

    //! Destroys the connection.
    virtual void Destroy();

    //! Closes the connection
    void Close();

    //! Get the output stream associated with the socket
    std::ostream &GetOutputStream() const { return *clientOutput; }

private:
    //! The server parenting this connection
    SEvServer * pServer;

    //! The socket for the connection
    int connSocket;

    //! The buffer stream for the socket
    SSocketBuff *       socketBuff;

    //! output stream for the socket
    std::ostream *      clientOutput;
};


#endif

