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
        clientInput(new std::istream(socketBuff)),
        clientOutput(new std::ostream(socketBuff)),
        createdAt(time(NULL)),
        connState(STATE_READING),
        pReadBuffer(NULL),
        bufferLength(0),
        pCurrPos(NULL),
        pBuffEnd(NULL)
{
    SLogger::Get()->Log("DEBUG: Creating Connection [%x], Socket: %d....\n", this, sock);
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
    SLogger::Get()->Log("DEBUG: Destroying connection [%x]....\n", this);
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
    if (pReadBuffer != NULL)
        delete [] pReadBuffer;
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
        if (result != 0)
        {
            SLogger::Get()->Log("ERROR: close error [%x]/[%d]: %s\n", this, errno, strerror(errno));
        }
        connSocket = -1;
    }
}

/**************************************************************************************
*   \brief  Ensures the read buffer is atleast this big in size.
*
*   \version
*       - Sri Panyam  09/07/2009
*         Created
**************************************************************************************/
int SConnection::RefillBuffer(char *&pOutCurrPos, char *&pOutBuffEnd)
{
    const int MAXBUF = 2048;
    if (pReadBuffer == NULL)
    {
        bufferLength = MAXBUF;
        pReadBuffer = new char[bufferLength];
        pCurrPos = pBuffEnd = pReadBuffer;
    }

    if (pCurrPos >= pBuffEnd)
    {
        int buffLen = read(Socket(), pReadBuffer, MAXBUF);
        if (buffLen <= 0)
        {
            if (buffLen == 0)
            {
                CloseSocket();
            }
            return buffLen;
        }

        pCurrPos = pReadBuffer;
        pBuffEnd = pReadBuffer + buffLen;
    }

    pOutCurrPos = pCurrPos;
    pOutBuffEnd = pBuffEnd;
    return pOutBuffEnd - pOutCurrPos;
}

