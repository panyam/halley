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
 *  A connection is a stream of several messages.  Each message is
 *  processed before the next message is read.  This assumptions means that
 *  we can leave data in the socket until required instead of fetching it
 *  and queuing it locally and THEN sending it off to be processed (which
 *  is the M.O. at 0.2.0).
 *
 *  The following states are required:
 *
 *  STATE_IDLE          -   This means the connection is at the "Start"
 *                          stage and a new request can be read - however
 *                          no data is available yet
 *
 *  STATE_READING       -   Connection is a state where data can be (if just 
 *                          accepted) or is being read.  At this point the
 *                          reader stage can read protocol specific
 *                          messages.
 *
 *  STATE_PROCESSING    -   At this stage a message that has been read in
 *                          the previous state is being processed by other 
 *                          stages/modules.
 *
 *  STATE_FINISHED      -   Here, a request has completed being handled,
 *                          and if data is available it will go ot the
 *                          reading stage, otherwise to the idle stage.
 *
 *  STATE_PEER_CLOSED   -   The read end of the connection is closed.  No
 *                          more reads can happen on the connection - but
 *                          writes are ok.
 *
 *  STATE_CLOSED        -   The connection can closed and freed and/or
 *                          reclaimed by the server at any time it sees fit.  
 *                          At or after this state a connection should not be 
 *                          used by other stages.
 *****************************************************************************/
class SConnection : public SJob
{
public:
    enum
    {
        STATE_IDLE,
        STATE_READING,
        STATE_PROCESSING,
        STATE_FINISHED,
        STATE_PEER_CLOSED,
        STATE_CLOSED,

        STATE_COUNT // Number of available states
    };

public:
    //! Creates a new connection
    SConnection(SEvServer *pSrv, int sock);

    //! Destroys the connection object
    virtual ~SConnection();

    //! Get the socket associated with the connection
    int Socket() { return connSocket; }

    //! Get the socket associated with the connection
    SEvServer *Server() { return pServer; }

    //! Get the connection state
    int GetState() const { return connState; }

    //! Get the connection state
    void SetState(int newState);

    //! Reads data from the socket
    int RefillBuffer(char *&pOutCurrPos, char *&pOutBuffEnd);

    //! Read n-bytes from the connection
    int ReadData(char *buffer, int nbytes);

    //! Writes data to the connection
    int WriteData(const char *buffer, int length);

protected:
    //! Closes the underlying socket
    void CloseSocket();

private:
    //! The server parenting this connection
    SEvServer *         pServer;

    //! The socket for the connection
    int                 connSocket;

    //! Time of connection acceptance
    time_t              createdAt;

    //! Connection state
    int                 connState;

public:
    //! Read buffers
    char *              pReadBuffer;
    size_t              bufferLength;
    char *              pCurrPos;
    char *              pBuffEnd;

    //! If data has been read then this is false
    bool                dataConsumed;
};

#endif

