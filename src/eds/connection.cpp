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
    SLogger::Get()->Log(0, "DEBUG: Creating Connection [%x], Socket: %d....\n", this, sock);
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
    SLogger::Get()->Log(0, "DEBUG: Destroying connection [%x]....\n", this);
    CloseSocket();
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

/**************************************************************************************
*   \brief  Closes the socket
*
*   \version
*       - Sri Panyam  08/07/2009
*         Created
**************************************************************************************/
void SConnection::CloseSocket()
{
    SetState(SConnection::STATE_CLOSED);
    if (connSocket > 0)
    {
        // shutdown(connSocket, SHUT_RDWR);
        int result = close(connSocket);
        if (result != NULL)
        {
            SLogger::Get()->Log(0, "ERROR: close error [%x]/[%d]: %s\n", this, errno, strerror(errno));
        }
        connSocket = -1;
    }
}

