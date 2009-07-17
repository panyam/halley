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
 *  \file   readerstage.cpp
 *
 *  \brief  The stage that asynchronously reads requests/messages of the
 *  socket - this can be inherited to read protocol specific messages.
 *
 *  \version
 *      - S Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#include "connection.h"
#include "server.h"
#include "readerstage.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <iostream>

const int MAXBUF = 2048;

// Creates a message reader stage.
SReaderStage::SReaderStage(const SString &name, int numThreads) : SStage(name, numThreads)
{
}

//! Destroys reader data
SReaderStage::~SReaderStage()
{
}

// Read bytes
bool SReaderStage::SendEvent_ReadRequest(SConnection *pConnection)
{
    return QueueEvent(SEvent(EVT_READ_REQUEST, pConnection));
}

//! Handles "read request" events.
//
// Will call the RequestHandler stage when a complete request has been read.
//
// The previous model was to read requests and queue them in.
void SReaderStage::HandleEvent(const SEvent &event)
{
    if (event.evType == EVT_READ_REQUEST)
    {
        HandleReadRequestEvent(event);
    }
}

//! Just deals with read_request event
void SReaderStage::HandleReadRequestEvent(const SEvent &event)
{
    // The connection currently being processed
    SConnection *pConnection    = (SConnection *)(event.pSource);
    void *pReaderState          = pConnection->GetStageData(this);

    if (pConnection->GetState() == SConnection::STATE_IDLE)
    {
        pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_READING);
        ResetStageData(pReaderState);
    }

    // Do nothing if we are processing a request - leave the data in the
    // socket buffer, but flag as readable so when we get to IDLE
    // stage, we can use the readable flag to see if there is more data
    // available for the next request
    pConnection->dataConsumed  = false;

    // in the reading state, we can read data till the next complete
    // "message" has been read...
    while (pConnection->GetState() == SConnection::STATE_READING)
    {
        if (pConnection->pReadBuffer == NULL)
        {
            pConnection->bufferLength   = MAXBUF;
            pConnection->pReadBuffer    = new char[pConnection->bufferLength];
            pConnection->pCurrPos       = pConnection->pReadBuffer;
            pConnection->pBuffEnd       = pConnection->pReadBuffer;
        }

        if (pConnection->pCurrPos >= pConnection->pBuffEnd)
        {
            int buffLen = ReadData(pConnection, pConnection->pReadBuffer, MAXBUF);
            if (buffLen <= 0)
                return ;

            pConnection->pCurrPos = pConnection->pReadBuffer;
            pConnection->pBuffEnd = pConnection->pReadBuffer + buffLen;
        }

        void *pRequest = AssembleRequest(pConnection->pCurrPos, pConnection->pBuffEnd, pReaderState);
        if (pRequest != NULL)
        {
            // set data consumed to false to indicate that there may be
            // data on the socket that we may have not dealt with yet so
            // that once a connection goes into the FINISHED state (eg as a
            // result of data being written out), the server can send a
            // ReadRequest if this is false after moving a connection to
            // the IDLE state.
            pConnection->dataConsumed = false;

            // send it of the next stage, also at this stage we have to
            // update how much data has been read
            pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_PROCESSING);

            // sends request to be handled by the next stage
            HandleRequest(pConnection, pRequest);
        }
    }
}

//! Reads data from the connection
int SReaderStage::ReadData(SConnection *pConnection, char *buffer, int nbytes)
{
    int buffLen = read(pConnection->Socket(), buffer, nbytes);
    if (buffLen <= 0)
    {
        if (buffLen == 0)
        {
            // end of file
            SLogger::Get()->Log("WARNING: read EOF reached\n\n");
            pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_PEER_CLOSED);
        }
        else if (errno == EAGAIN)
        {
            // non blocking io - so quit till more data is available
            SLogger::Get()->Log("DEBUG: read error EAGAIN = [%d]: %s\n\n", errno, strerror(errno));

            // clear readable flag since there is no more data available
            pConnection->dataConsumed = true;
        }
        else if (errno == ECONNRESET)
        {
            // non blocking io - so quit till more data is available
            SLogger::Get()->Log("DEBUG: read error ECONNRESET = [%d]: %s\n\n", errno, strerror(errno));
            pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_CLOSED);
        }
        else
        {
            SLogger::Get()->Log("ERROR: read error [%d]: %s\n\n", errno, strerror(errno));
        }
    }
    return buffLen;
}
