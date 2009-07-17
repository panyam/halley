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
 *  \class  multiserver.h
 *
 *  \brief  A server that allows multiple clients to be handled simultaneously.
 *
 *  \version
 *      - S Panyam      17/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SMULTI_SERVER_H_
#define _SMULTI_SERVER_H_

#include "net/server.h"
#include "thread/mutex.h"

class SConnFactory;
class SConnHandler;

//*****************************************************************************
/*!
 *  \class  SMultiServer
 *
 *  \brief Superclass of all multi-client servers.
 *
 *****************************************************************************/
class SMultiServer : public SServer
{
public:
    //! Constructor
    SMultiServer(int port_ = -1) : SServer(port_), connFactory(NULL) { }

    //! Destructor
    ~SMultiServer();

    //! Sets the connection pool to use.
    // TODO: should this be allowed when the server is running?
    void SetConnectionFactory(SConnFactory *factory) { connFactory = factory; }

    //! Called by the handler when it has finished
    virtual void HandlerFinished(SConnHandler *pHandler);

protected:
    // Called to stop the task.
    virtual int RealStop();

    // handle a new connection
    virtual void HandleConnection(int clientSocket);

protected:
    SConnFactory *          connFactory;

private:
    //! Declared functions but not implemented.
    SMultiServer(const SServer &);
    SMultiServer & operator=(const SServer &);

private:
    //! Child handlers currently handling connections
    std::list<SConnHandler *>   connHandlers;

    //! Mutex to lock children list when necessary
    SMutex                      handlerListMutex;
};

#endif

