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
    //! Creates a module data object.
    SHttpWriterState() : pCurrRequest(NULL) { }

public:
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

public:
    // Different states the writer can be in to 
    // accomodate asynchronous writes
    enum
    {
        STATE_IDLE,
        STATE_WRITING_HEADERS,
        STATE_WRITING_BODY,
    }

    //! Current writer state
    int     currState;

    //! How many bytes in the current state has been written?
    int     bytesWritten;

    //! The complete output of the headers that are being written - only
    //  valid if we are in the WRITING_HEADERS state
    SString currPayLoad;

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
    int                 result          = 0;
    int                 sock            = pConnection->Socket();

    pWriterState->ResumeWriting(pConnection, pBodyPart);
}

int SHttpWriterStage::WriteBodyPart(SConnection *       pConnection,
                                     SHttpWriterState * pWriterState,
                                     SBodyPart *        pBodyPart)
{
    int             result      = 0;
    SHttpRequest *  pRequest    = pWriterState->Request();
    SHttpResponse * pResponse   = pRequest->Response();
    SHeaderTable &reqHeaders    = pRequest->Headers();
    SHeaderTable &respHeaders   = pResponse->Headers();
    SString transferEncoding(respHeaders.Header("Transfer-Encoding"));

    if (pBodyPart->Type() == SBodyPart::BP_CONTENT_FINISHED ||
        pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION)
    {
        // reset last BP sent as no more packets will 
        // be sent for this request
        bool closeConnection        = reqHeaders.CloseConnection();
        pWriterState->nextBP          = 0;
        pWriterState->nextBPToSend    = 0;

        // remove and destroy the request from the queue
        // Note this destroys pRequest - 
        // dont use pRequest or Request() after this
        pWriterState->DestroyRequest();

        // do nothing - close connection only if close header found
        if (closeConnection ||
            pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION ||
            pConnection->GetState() >= SConnection::STATE_PEER_CLOSED)
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
        result = pBodyPart->WriteBodyToFD(pConnection->Socket());
        pWriterState->nextBPToSend++;
    }

    // now delete the body part - its no longer needed!
    delete pBodyPart;
    return result;
}

void SHttpWriterState::ResumeWriting(SConnection *pConnection, SBodyPart *pBodyPart)
{
    int                 result          = 0;
    int                 sock            = pConnection->Socket();
    if (pBodyPart != NULL)
    {
        PutBodyPart(pBodyPart);
    }

    while (true)
    {
        if (currState == SHttpWriterState::STATE_IDLE)
        {
            if (pCurrBodyPart == NULL)
            {
                pCurrBodyPart = NextBodyPart();
            }

            SHttpRequest *  pRequest    = Request();
            SHttpResponse * pResponse   = pRequest->Response();
            SHeaderTable &  respHeaders = pResponse->Headers();
            stringstream    sstr;

            pResponse->WriteHeaderLineToStream(sstr);
            pResponse->WriteHeader(sstr);
            respHeaders.WriteToStream(sstr);

            currState == SHttpWriterState::STATE_WRITING_HEADERS;
            bytesWritten = 0;
            currPayload = sstr.str();
        }
        else if (currState == SHttpWriterState::STATE_WRITING_HEADERS)
        {
            int numWritten = send(sock, currPayload.c_str() + bytesWritten, currPayload.size() - bytesWritten, MSG_NOSIGNAL);
            if (numWritten < 0)
            {
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_CLOSED);
                }
                else if (errno == EAGAIN)
                {
                    // come back again
                    return ;
                }
                else
                {
                    assert("Some other error" && false);
                }
                return ;
            }
            bytesWritten += numWritten;
            if (bytesWritten == currPayload.size())
            {
                // done go to the next stage
                currState       = SHttpWriterState::STATE_WRITING_BODY;
                bytesWritten    = 0;
                currPayload     = "";
                pCurrBodyPart   = NULL;
                pCurrRequest    = NULL;
            }
        }
    }

    else
    {
    }

    else if (event.evType == EVT_WRITE_BODY_PART)
    {
        // first send the headers regardless of whether there are any 
        // body parts so it is done with
        if (nextBPToSend == 0)
        {
            SHttpRequest *  pRequest    = Request();
            SHttpResponse * pResponse   = pRequest->Response();
            SHeaderTable &  respHeaders = pResponse->Headers();
            // write headers
            SString transferEncoding(respHeaders.Header("Transfer-Encoding"));
            if (strcasecmp(transferEncoding.c_str(), "chunked") == 0)
            {
                respHeaders.RemoveHeader("Content-Length");
            }

            respHeaders.Lock();

            result = pResponse->WriteHeaderLineToFD(pConnection->Socket());
            if (result >= 0)
            {
                result = respHeaders.WriteToFD(pConnection->Socket());
                SLogger::Get()->Log("DEBUG: ===============================\n\n");
            }
        }

        if (result >= 0 || pBodyPart != NULL)
        {
            pBodyPart               = PutAndGetBodyPart(pBodyPart);
            while (pBodyPart != NULL)
            {
                result = WriteBodyPart(pConnection, pBodyPart);
                if (result >= 0)
                {
                    pBodyPart = pWriterState->NextBodyPart();
                }
                else
                {
                    pBodyPart = NULL;
                }
            }
        }

        if (result < 0)
        {
            SLogger::Get()->Log("TRACE: send error: [%d], ENOMEM/EBADF = [%d]/[%d] - [%s]\n", errno, ENOMEM, EBADF, strerror(errno));
            if (errno == EPIPE || errno == ECONNRESET)
            {
                pConnection->Server()->SetConnectionState(pConnection, SConnection::STATE_CLOSED);
            }
            else if (errno == EAGAIN)
            {
                assert("Asynch writes not yet implemented" && false);
            }
            else
            {
                assert("Some other error" && false);
            }
        }
    }
}
