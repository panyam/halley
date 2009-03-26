//*****************************************************************************
/*!
 *  \file   tcpserver.h
 *
 *  \brief
 *
 *  \version
 *      - Sri Panyam      10/02/2009
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

#include <iostream>
#include <fstream>

#include <list>
#include <string>
#include <vector>

#include "thread/task.h"
#include "thread/mutex.h"

class SConnFactory;
class SConnHandler;

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
    static int setnonblocking(int fd);

public:
    //! Constructor
    SServer(int port_ = -1) : connFactory(NULL), serverPort(port_) { }

    //! Destructor
    ~SServer();

    //! Return the server's port
    inline int GetPort() { return this->serverPort; }

    //! Set the server's port
    inline void SetPort(int port) { this->serverPort = port; }

    //! Sets the connection pool to use.
    // TODO: should this be allowed when the server is running?
    void SetConnectionFactory(SConnFactory *factory) { connFactory = factory; }

    //! Called by the handler when it has finished
    virtual void HandlerFinished(SConnHandler *pHandler);

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

protected:
    SConnFactory *          connFactory;

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

private:
    //! Child handlers currently handling connections
    std::list<SConnHandler *>   connHandlers;

    //! Mutex to lock children list when necessary
    SMutex                      handlerListMutex;
};

#endif

