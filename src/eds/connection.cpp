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
        createdAt(time(NULL)),
        connState(STATE_IDLE),
        pReadBuffer(NULL),
        bufferLength(0),
        pCurrPos(NULL),
        pBuffEnd(NULL),
        dataConsumed(false)
{
    SLogger::Get()->Log("\nTRACE: Creating Connection [%x], Socket: %d....\n", this, sock);
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
    SLogger::Get()->Log("TRACE: Destroying Connection [%x], Socket: %d....\n\n", this, connSocket);
    CloseSocket();
    if (pReadBuffer != NULL)
        delete [] pReadBuffer;
}

/**************************************************************************************
*   \brief  Sets the connection state
*
*   \version
*       - Sri Panyam  08/07/2009
*         Created
**************************************************************************************/
void SConnection::SetState(int state)
{
    connState = state;
    if (state == STATE_PEER_CLOSED)
    {
        // shutdown(connSocket, SHUT_RD);
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

//! Writes data to the connection
int SConnection::WriteData(const char *buffer, int length)
{
    int numWritten = send(Socket(), buffer, length, MSG_NOSIGNAL);
    if (numWritten < 0)
    {
        if (errno == EPIPE || errno == ECONNRESET)
        {
            Server()->SetConnectionState(this, SConnection::STATE_CLOSED);
        }
        else if (errno == EAGAIN)
        {
            SLogger::Get()->Log("DEBUG: Write Later...");
        }
        else
        {
            SLogger::Get()->Log("TRACE: send error: [%d] - [%s]\n",
                                errno, strerror(errno));
            assert("Some other error" && false);
        }
    }
    return numWritten;
}

//! Reads data from the connection
int SConnection::ReadData(char *buffer, int nbytes)
{
    int buffLen = read(Socket(), buffer, nbytes);
    if (buffLen <= 0)
    {
        if (buffLen == 0)
        {
            // end of file
            SLogger::Get()->Log("WARNING: read EOF reached\n\n");
            Server()->SetConnectionState(this, SConnection::STATE_PEER_CLOSED);
        }
        else if (errno == EAGAIN)
        {
            // non blocking io - so quit till more data is available
            SLogger::Get()->Log("DEBUG: read error EAGAIN = [%d]: %s\n\n", errno, strerror(errno));

            // clear readable flag since there is no more data available
            dataConsumed = true;
        }
        else if (errno == ECONNRESET)
        {
            // non blocking io - so quit till more data is available
            SLogger::Get()->Log("DEBUG: read error ECONNRESET = [%d]: %s\n\n", errno, strerror(errno));
            Server()->SetConnectionState(this, SConnection::STATE_CLOSED);
        }
        else
        {
            SLogger::Get()->Log("ERROR: read error [%d]: %s\n\n", errno, strerror(errno));
        }
    }
    return buffLen;
}

