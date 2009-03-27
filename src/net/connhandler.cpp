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
 *  \brief  Superclass of all connection handlers.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "net/server.h"
#include "net/connhandler.h"
#include "net/sockbuff.h"
#include "json/json.h"


/**************************************************************************************
*   \brief  Constructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SConnHandler::SConnHandler() : 
    pServer(NULL),
    clientSocket(-1),
    clientInput(NULL),
    clientOutput(NULL)
{
}

/**************************************************************************************
*   \brief  Destructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SConnHandler::~SConnHandler()
{
    Reset();
}

/**************************************************************************************
*   \brief  Initialises the client.
*
*   \param  SServer *pServer    Parent server initialising this client.
*   \param  int sock            The socket for the client.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
void SConnHandler::Init(SServer *server, int sock)
{
    pServer                 = server;
    clientSocket            = sock;
    SSocketBuff * strbuff   = new SSocketBuff(sock);
    clientInput             = new std::istream(strbuff);
    clientOutput            = new std::ostream(strbuff);
}

/**************************************************************************************
*   \brief  Clears input and output streams.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
void SConnHandler::Reset()
{
    if (clientInput != NULL)
    {
        delete clientInput;
        clientInput = NULL;
    }

    if (clientOutput != NULL)
    {
        delete clientOutput;
        clientOutput = NULL;
    }

    if (clientSocket >= 0)
    {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }
}

/**************************************************************************************
*   \brief  Stops the client
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
int SConnHandler::RealStop()
{
    clientStopped = true;
    if (clientSocket >= 0)
    {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }

    return 0;
}

