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
 *  \file   connection.cpp
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

#include "connection.h"
#include "server.h"
#include "stage.h"
#include "handler.h"

/**************************************************************************************
*   \brief  Creates a new connection object and required members.
*
*   \version
*       - Sri Panyam  04/03/2009
*         Created
**************************************************************************************/
SConnection::SConnection(SEvServer *pSrv, int sock) : 
        pServer(pSrv),
        connSocket(sock),
        socketBuff(new SSocketBuff(sock)),
        clientOutput(new std::ostream(socketBuff)),
        createdAt(time(NULL)),
        connState(STATE_READING)
{
    SLogger::Get()->Log(0, "Creating connection ....\n");
}

/**************************************************************************************
*   \brief  Destroys the connection and associated data.
*
*   \version
*       - Sri Panyam  04/03/2009
*         Created
**************************************************************************************/
SConnection::~SConnection()
{
    SLogger::Get()->Log(0, "Destroying connection ....\n");
    if (connSocket > 0)
    {
        // shutdown(connSocket, SHUT_RDWR);
        close(connSocket);
        connSocket = -1;
    }
    if (socketBuff != NULL)
    {
        delete socketBuff;
        socketBuff = NULL;
    }
    if (clientOutput != NULL)
    {
        delete clientOutput;
        clientOutput = NULL;
    }
}

