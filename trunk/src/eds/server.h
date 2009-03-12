//*****************************************************************************
/*!
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
    SEvServer(int port_, SHttpReaderStage*pReqReader_)
        :   serverPort(port_),
            pRequestReader(pReqReader_)
    {
    }

    //! Destructor
    ~SEvServer();

    //! Return the server's port
    inline int GetPort() { return this->serverPort; }

    //! Set the server's port
    inline void SetPort(int port) { this->serverPort = port; }

    //! Sets an event stage for a given name
    void        SetStage(const std::string &name, SStage *pStage);

    //! Sets an event stage for a given name
    SStage *    GetStage(const std::string &name);

    //! Called by a connection when it is done with.
    void        ConnectionComplete(SConnection *pConnection);

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

private:
    //! Declared functions but not implemented.
    SEvServer(const SEvServer &);
    SEvServer & operator=(const SEvServer &);

private:
    //! The server port
    unsigned            serverPort;

    //! The server socket
    int                 serverSocket;

private:
    //! The request reader stage
    SHttpReaderStage *              pRequestReader;

    //! All other stages by name
    std::map<std::string, SStage *> eventStages;
};

#endif

