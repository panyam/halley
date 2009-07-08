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
#include "readerstage.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <iostream>

const int MAXBUF = 8192;

// Creates a message reader stage.
SReaderStage::SReaderStage(const SString &name, int numThreads)
:
    SStage(name, numThreads),
    pReadBuffer(new char[MAXBUF]),
    pCurrPos(pReadBuffer),
    pBuffEnd(pReadBuffer)
{
}

//! Destroys reader data
SReaderStage::~SReaderStage()
{
    if (pReadBuffer != NULL)
        delete [] pReadBuffer;
}

// Read bytes
void SReaderStage::ReadSocket(SConnection *pConnection)
{
    QueueEvent(SEvent(EVT_BYTES_RECIEVED, pConnection));
}

//! Called when a connection is going to be destroyed so we can do our
// connection specific cleanup.
void SReaderStage::JobDestroyed(SJob *pJob)
{
    if (pJob != NULL)
    {
        void *pStageData = pJob->GetStageData(this);
        if (pStageData != NULL)
        {
            DestroyReaderState(pStageData);
            pJob->SetStageData(this, NULL);
        }
    }
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
    if (pReaderState == NULL)
    {
        pReaderState = CreateReaderState();
        pConnection->SetStageData(this, pReaderState);
        pConnection->AddListener(this);
    }

    if (pConnection->GetState() == SConnection::STATE_ACCEPTED)
    {
        // we are in the accepted state - so go to the READING state
        pConnection->SetState(SConnection::STATE_READING);
    }

    // in the reading state, we can read data till the next complete
    // "message" has been read...
    if (pConnection->GetState() == SConnection::STATE_READING)
    {
        while (true)
        {
            // refill read buffer if necessary
            if (pCurrPos >= pBuffEnd)
            {
                int buffLen = read(pConnection->Socket(), pReadBuffer, MAXBUF);
                if (buffLen < 0)
                {
                    if (errno == EAGAIN)
                    {
                        // non blocking io - so quit till more data is available
                        SLogger::Get()->Log(0, "DEBUG: read EAGAIN %d [%d]: %s\n\n", EAGAIN, errno, strerror(errno));
                    }
                    else
                    {
                        SLogger::Get()->Log(0, "ERROR: read error [%d]: %s\n\n", errno, strerror(errno));
                        // close connection? destroy it?
                    }
                    return ;
                }
                else if (buffLen == 0)
                {
                    // end of file - close connection?
                    SLogger::Get()->Log(0, "WARNING: read EOF reached\n\n");
                }
                pCurrPos = pReadBuffer;
                pBuffEnd = pReadBuffer + buffLen;
            }

            void *pRequest;
            if ((pRequest = AssembleRequest(pCurrPos, pBuffEnd)) != NULL)
            {
                // send it of the next stage, also at this stage we have to
                // update how much data has been read
                pConnection->SetState(SConnection::STATE_PROCESSING);

                // send the request off to the handler stage
                // pHandlerStage->HandleRequest(pConnection, pRequest);
            }
        }
    }
}

//! Processes bytes int he current buffer
void *SReaderStage::AssembleRequest(char *&pStart, char *&pLast)
{
    return NULL;
}

