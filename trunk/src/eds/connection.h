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
 *  STATE_READING       -   Connection is a state where data can be (if just 
 *                          accepted) or is being read.  At this point the
 *                          reader stage can read protocol specific
 *                          messages.
 *
 *  STATE_PROCESSING    -   At this stage a message that has been read in
 *                          the previous state is being processed by other 
 *                          stages/modules.
 *
 *  STATE_WRITING       -   Message that has been processed in the PROCESSING 
 *                          state can have its response written in this state.
 *                          The problem is if the writing of a response
 *                          happens here, then we may have to do things
 *                          like buffering of messages and duplicate
 *                          copying between threads.  To avoid this we
 *                          should perhaps let the stage do its own
 *                          serialization (more than once if necessary) in
 *                          the processing stage.
 *
 *  STATE_FINISHED      -   Processing of a message has completed.  At this
 *                          stage a connection can be closed or left open.
 *                          If the connection is left open, it goes back to
 *                          the STATE_READING state.
 *
 *  STATE_CLOSED        -   The connection is closed.  It can be freed
 *                          and/or reclaimed by the server.  At or after
 *                          this state a connection should not be used by
 *                          other stages.
 *****************************************************************************/
class SConnection : public SJob
{
public:
    enum
    {
        STATE_READING,
        STATE_PROCESSING,
        // STATE_WRITING,
        // STATE_FINISHED,
        STATE_CLOSED,
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

    //! Get the output stream associated with the socket
    std::ostream &GetOutputStream() const { return *clientOutput; }

    //! Get the output stream associated with the socket
    std::istream &GetInputStream() const { return *clientInput; }

    //! Get the connection state
    int GetState() const { return connState; }

    //! Get the connection state
    void SetState(int newState) { connState = newState; }

    //! Closes the underlying socket
    void CloseSocket();

    //! Reads data from the socket
    int RefillBuffer(char *&pOutCurrPos, char *&pOutBuffEnd);

private:
    //! The server parenting this connection
    SEvServer *         pServer;

    //! The socket for the connection
    int                 connSocket;

    //! The buffer stream for the socket
    SSocketBuff *       socketBuff;

    //! input stream for the socket
    std::istream *      clientInput;

    //! output stream for the socket
    std::ostream *      clientOutput;

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
};

#endif

