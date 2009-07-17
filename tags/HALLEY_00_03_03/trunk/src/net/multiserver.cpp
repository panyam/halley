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
 *  \file   multiserver.cpp
 *
 *  \brief  Superclass of all multithreaded servers.
 *
 *  \version
 *      - S Panyam      17/07/2009
 *        Created
 *
 *****************************************************************************/

#include "net/multiserver.h"
#include "net/connhandler.h"
#include "net/connpool.h"

//*****************************************************************************
/*!
 *  \brief  Called to stop the server.
 *
 *  \version
 *      - Sri Panyam      17/07/2009
 *        Created.
 *
 *****************************************************************************/
int SMultiServer::RealStop()
{
    SServer::RealStop();

    // Stop child threads first
    SMutexLock    mutexLock(handlerListMutex);
    for (std::list<SConnHandler *>::iterator iter=connHandlers.begin();
            iter != connHandlers.end();
            ++iter)
    {
        // stop the client first
        if ((*iter)->Running())
        {
            (*iter)->Stop();
        }

        // then the client
        delete (*iter);
    }

    // clear client list
    connHandlers.clear();

    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Called by the handler when it has finished in asynch mode.
 *
 *  \param  SConnHandler *  The handler that has just finished.
 *
 *  \version
 *      - Sri Panyam      17/07/2009
 *        Created.
 *
 *****************************************************************************/
void SMultiServer::HandlerFinished(SConnHandler *pHandler)
{
    std::cerr << "Done Handling Connection: " << pHandler->Socket() << std::endl;
    pHandler->Reset();
    if (connFactory != NULL)
        connFactory->ReleaseHandler(pHandler);
}

//*****************************************************************************
/*!
 *  \brief  Handles a connection request by a client.
 *
 *  \param  int clientSocket    The client socket being handled.
 *
 *  \version
 *      - Sri Panyam      17/07/2009
 *        Created.
 *
 *****************************************************************************/
void SMultiServer::HandleConnection(int clientSocket)
{
    bool handled = false;
    std::cerr << "Handling Connection: " << clientSocket << std::endl;
    if (connFactory != NULL)
    {
        SConnHandler *handler = connFactory->NewHandler();
        if (handler != NULL)
        {
            handler->Init(this, clientSocket);
            handled = handler->HandleConnection();
            if (!handled)
            {
                // means handler is in asynchronous mode - so add it to our
                // list so clients, so the handler will have to explicitly
                // call back when it is done with
                SMutexLock locker(handlerListMutex);

                // TODO: assert if handler is already in the handler list!
                connHandlers.push_back(handler);
            }
            else
            {
                connFactory->ReleaseHandler(handler);
            }
        }
    }

    if (handled)
    {
        SServer::HandleConnection(clientSocket);
    }
}

