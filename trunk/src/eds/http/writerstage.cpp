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
 *  \file   writerstage.cpp
 *
 *  \brief  A stage that writes the request to the socket.  The modules
 *  dont/shouldnt care about this.  When there is no longer a module to
 *  send to, the http handler stage will automatically send the data to
 *  this stage.
 *
 *  \version
 *      - S Panyam      14/07/2009
 *        Created
 *
 *****************************************************************************/

#include "eds/server.h"
#include "eds/connection.h"
#include "writerstage.h"
#include "readerstage.h"
#include "httpmodule.h"
#include "request.h"
#include "response.h"

//! Maintains request specific module data.
class SHttpWriterState : public SHttpModuleData
{
public:
    // Different states the writer can be in to 
    // accomodate asynchronous writes
    enum
    {
        STATE_IDLE,
        STATE_WRITING_HEADERS,
        STATE_WRITING_BODY,
    };

public:
    //! Creates a module data object.
    SHttpWriterState() :
        currState(STATE_IDLE),
        bytesWritten(0),
        pCurrBodyPart(NULL),
        pCurrRequest(NULL) { }

    virtual void Reset()
    {
        SHttpModuleData::Reset();
        // reset reader state
        currState           = STATE_IDLE;
        bytesWritten        = 0;
        currPayload         = "";
        pCurrBodyPart       = NULL;
        pCurrRequest        = NULL;
    }

    //! Adds a new request to the queue.
    virtual void SetRequest(SHttpRequest *pReq) { pCurrRequest = pReq; }

    //! Destroys the current request
    virtual void DestroyRequest() { pCurrRequest = NULL; }

    //! Current request being handled
    inline SHttpRequest *Request() { return pCurrRequest; }

    //! Resumes writing of data
    void ResumeWriting(SConnection *pConnection, SBodyPart *pBodyPart);

public:
    //! Current writer state
    int     currState;

    //! How many bytes in the current state has been written?
    int     bytesWritten;

    //! The complete output of the headers that are being written - only
    //  valid if we are in the WRITING_HEADERS state
    SString currPayload;

    //! Current body part being written - only used in WRITING_BODY stage
    SBodyPart *pCurrBodyPart;

    //! Current requests being processed
    SHttpRequest *  pCurrRequest;
};

// Creates a new file io helper stage
SHttpWriterStage::SHttpWriterStage(const SString &name, int numThreads) : SWriterStage(name, numThreads)
{
}

//! Creates a new reader state object
void *SHttpWriterStage::CreateStageData()
{
    return new SHttpWriterState();
}

//! Destroys reader state objects
void SHttpWriterStage::DestroyStageData(void *pReaderState)
{
    if (pReaderState != NULL)
        delete ((SHttpWriterState*)pReaderState);
}

//! write a body part out
bool SHttpWriterStage::SendEvent_WriteBodyPart(SConnection *pConnection, SBodyPart *pBodyPart)
{
    return QueueEvent(SEvent(EVT_WRITE_BODY_PART, pConnection, pBodyPart));
}

//! Re orders and sends out http body parts to the socket
void SHttpWriterStage::HandleEvent(const SEvent &event)
{
    SConnection *       pConnection     = (SConnection *)(event.pSource);
    SHttpWriterState *  pWriterState    = (SHttpWriterState *)pConnection->GetStageData(this);
    SBodyPart *         pBodyPart       = (SBodyPart *)(event.pData);

    pWriterState->ResumeWriting(pConnection, pBodyPart);
}

void SHttpWriterState::ResumeWriting(SConnection *pConnection, SBodyPart *pBodyPart)
{
    int numWritten = 0;

    if (pBodyPart != NULL)
    {
        // A new body needs to be sent out - 
        // if a body is being written then queue this...
        PutBodyPart(pBodyPart);
        if (currState == STATE_IDLE)
        {
            assert("Why is current body not NULL??" && pCurrBodyPart == NULL);
        }
    }

    while (true)
    {
        if (currState == SHttpWriterState::STATE_IDLE)
        {
            if (pCurrBodyPart == NULL)
            {
                pCurrBodyPart = NextBodyPart();
                if (pCurrBodyPart == NULL)
                {
                    // no more body parts so just quit and come back later
                    return ;
                }
                SetRequest(pCurrBodyPart->ExtraData<SHttpRequest *>());
            }

            if (nextBPToSend == 0)
            {
                // first body part in the chain
                SHttpResponse * pResponse   = pCurrRequest->Response();
                SHeaderTable &  respHeaders = pResponse->Headers();
                stringstream    sstr;

                // write headers
                SString transferEncoding(respHeaders.Header("Transfer-Encoding"));
                if (strcasecmp(transferEncoding.c_str(), "chunked") == 0)
                {
                    respHeaders.RemoveHeader("Content-Length");
                }

                pResponse->WriteHeaderLineToStream(sstr);
                respHeaders.WriteToStream(sstr);
                respHeaders.Lock(); // no more changes allowed in response headers

                currState       = STATE_WRITING_HEADERS;
                bytesWritten    = 0;
                currPayload     = sstr.str();
            }
            else
            {
                currState       = STATE_WRITING_BODY;
                bytesWritten    = 0;
            }
        }
        else if (currState == STATE_WRITING_HEADERS)
        {
            int length      = currPayload.size() - bytesWritten;
            if ((numWritten = pConnection->WriteData(currPayload.c_str() + bytesWritten, length)) < 0)
                return ;
            bytesWritten += numWritten;
            if (numWritten == length)
            {
                // done go to the next stage
                currState       = SHttpWriterState::STATE_WRITING_BODY;
                bytesWritten    = 0;
                currPayload     = "";
            }
        }
        else if (currState == STATE_WRITING_BODY)
        {
            assert("Request Cannot be NULL" && pCurrRequest != NULL);

            if (pCurrBodyPart == NULL)
            {
                assert("On a new BP, bytesWritten MUST be 0" && bytesWritten == 0);
                pCurrBodyPart = NextBodyPart();
                if (pCurrBodyPart == NULL)
                {
                    // no more body parts so just quit and come back later
                    return ;
                }
                assert("Current request must be same as body's request" && pCurrRequest == pCurrBodyPart->ExtraData<SHttpRequest *>());
            }

            SHttpResponse * pResponse   = pCurrRequest->Response();
            SHeaderTable &  reqHeaders  = pCurrRequest->Headers();
            SHeaderTable &  respHeaders = pResponse->Headers();
            int             bpType      = pCurrBodyPart->Type();
            SString transferEncoding(respHeaders.Header("Transfer-Encoding"));

            if (bpType == SHttpMessage::HTTP_BP_CONTENT_FINISHED ||
                bpType == SHttpMessage::HTTP_BP_CLOSE_CONNECTION)
            {
                // reset last BP sent as no more packets will 
                // be sent for this request
                bool closeConnection    = reqHeaders.CloseConnection() ||
                                          bpType == SHttpMessage::HTTP_BP_CLOSE_CONNECTION;
                bytesWritten            = 0;
                nextBP                  = 0;
                nextBPToSend            = 0;
                currState               = STATE_IDLE;

                delete pCurrBodyPart;
                pCurrBodyPart   = NULL;

                // remove and destroy the request from the queue
                // Note this destroys pRequest - 
                // dont use pRequest or Request() after this
                DestroyRequest();

                // do nothing - close connection only if close header found
                if (closeConnection || pConnection->GetState() >= SConnection::STATE_PEER_CLOSED)
                {
                    // a connection is to be closed -
                    // the problem is regardless of how many threads we or other stages
                    // have, killing it here will pose sever problems.  So instead of
                    // killing, we flag it as being closed so nothing else uses this
                    // connection
                    pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_CLOSED);
                }
                else
                {
                    // tell the reader we are ready for more
                    // should we? or should we let the server take care of this?
                    pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_FINISHED);
                }
            }
            else // treat as normal message
            {
                int numWritten = 0;
                bool bytesLeft  = pCurrBodyPart->WriteToConnection(pConnection, numWritten, bytesWritten);
                if (numWritten < 0)
                    return ;

                bytesWritten += numWritten;
                if (!bytesLeft)
                {
                    // done go to the next body part, 
                    // but state remains the same
                    bytesWritten    = 0;
                    nextBPToSend++;
                    delete pCurrBodyPart;
                    pCurrBodyPart = NULL;
                }
            }
        }
        else
        {
            assert("Invalid state" && false);
        }
    }
}

