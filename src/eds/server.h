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
 *  \file   evserver.h
 *
 *  \brief
 *
 *  An event driven server where the emphasis is on non-blocking operations
 *  and events instead of blocks and waits.
 *
 *  \version
 *      - Sri Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_SERVER_H_
#define _SEVENT_SERVER_H_

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

#include "thread/thread.h"
#include "eds/fwd.h"
#include "eds/http/httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SEvServer
 *
 *  \brief Superclass of all servers.
 *
 *****************************************************************************/
class SEvServer : public STask
{
public:
    static int setnonblocking(int fd);

public:
    //! Constructor
    SEvServer(int port_, SReaderStage*pReqReader_ = NULL);

    //! Destructor
    ~SEvServer();

    //! Set the reader stage
    void SetReaderStage(SReaderStage* pReaderStage_ = NULL)
    {
        pReaderStage = pReaderStage_;
    }

    //! Set the writer stage
    void SetWriterStage(SWriterStage* pWriterStage_ = NULL)
    {
        pWriterStage = pWriterStage_;
    }

    //! Return the server's port
    inline int GetPort() { return this->serverPort; }

    //! Set the server's port
    inline void SetPort(int port) { this->serverPort = port; }

    //! Sets an event stage for a given name
    void        SetStage(const std::string &name, SStage *pStage);

    //! Sets an event stage for a given name
    SStage *    GetStage(const std::string &name);

    //! Creates a new connection
    SConnection *NewConnection(int clientSocket);

    //! Destroyes connections marked as closed
    void        CloseMarkedConnections();

    //! Marks a connection as closed
    void        MarkConnectionAsClosed(SConnection *pConnection);

    //! Peer has closed the connection - so remove the POLLIN
    void        PeerClosedConnection(SConnection *pConnection);

protected:
    // Called to stop the task.
    virtual int RealStop();

    // Main task method
    virtual int Run();

    // Creates a server socket 
    virtual int CreateSocket();

    // Perpares client socket 
    virtual int PrepareClientSocket(int clientSocket);

    // Binds the socket
    virtual int BindSocket();

    // Listens on the socket
    virtual int ListenOnSocket();

private:
    //! Declared functions but not implemented.
    SEvServer(const SEvServer &);
    SEvServer & operator=(const SEvServer &);

private:
    //! The server port
    unsigned            serverPort;

    //! The server socket
    int                 serverSocket;

    //! The epoll file descriptor
    int                 serverEpollFD;

    //! Number of fds epoll is handling currently
    int                 numEpollFDs;

private:
    //! The request reader stage
    SReaderStage *              pReaderStage;

    //! The writer stage
    SWriterStage *              pWriterStage;

    //! All other stages by name
    std::map<std::string, SStage *> eventStages;

    //! List of all connections
    std::set<SConnection *>         connections;

    //! List of connections that can be garbage collected
    std::list<SConnection *>        closedConnections;

    //! Mutex on the connection list
    SMutex                          connListMutex;
};

#endif

