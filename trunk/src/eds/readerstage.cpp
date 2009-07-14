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
    // The connection currently being processed
    SConnection *pConnection    = (SConnection *)(event.pSource);
    void *pReaderState          = pConnection->GetStageData(this);

    if (event.evType == EVT_READ_REQUEST)
    {
        const int MAXBUF = 2048;

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
                int buffLen = read(pConnection->Socket(), pConnection->pReadBuffer, MAXBUF);
                if (buffLen <= 0)
                {
                    if (buffLen == 0)
                    {
                        // end of file
                        SLogger::Get()->Log("WARNING: read EOF reached\n\n");
                        pConnection->SetState(SConnection::STATE_PEER_CLOSED);
                    }
                    else if (errno == EAGAIN)
                    {
                        // non blocking io - so quit till more data is available
                        SLogger::Get()->Log("DEBUG: read EAGAIN = [%d]: %s\n\n", errno, strerror(errno));
                    }
                    else
                    {
                        SLogger::Get()->Log("ERROR: read error [%d]: %s\n\n", errno, strerror(errno));
                    }
                    return ;
                }

                pConnection->pCurrPos = pConnection->pReadBuffer;
                pConnection->pBuffEnd = pConnection->pReadBuffer + buffLen;
            }

            void *pRequest = AssembleRequest(pConnection->pCurrPos, pConnection->pBuffEnd, pReaderState);
            if (pRequest != NULL)
            {
                // send it of the next stage, also at this stage we have to
                // update how much data has been read
                pConnection->SetState(SConnection::STATE_PROCESSING);

                // sends request to be handled by the next stage
                HandleRequest(pConnection, pRequest);
            }
        }
    }
}

