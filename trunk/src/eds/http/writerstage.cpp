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

class SHttpStageData : public SHttpModuleData
{
public:
    //! a new stage data
    SHttpStageData() : pCurrRequest(NULL) { }

    //! Adds a new request to the queue.
    virtual void SetRequest(SHttpRequest *pReq) { pCurrRequest = pReq; }

    //! Destroys the current request
    virtual void DestroyRequest() { pCurrRequest = NULL; }

    //! Current request being handled
    inline SHttpRequest *Request() { return pCurrRequest; }

protected:
    //! Current request being processed
    SHttpRequest *          pCurrRequest;
};

// Creates a new file io helper stage
SHttpWriterStage::SHttpWriterStage(const SString &name, int numThreads)
:
    SWriterStage(name, numThreads),
    pReaderStage(NULL)
{
}

//! Creates a new reader state object
void *SHttpWriterStage::CreateStageData()
{
    return new SHttpStageData();
}

//! Destroys reader state objects
void SHttpWriterStage::DestroyStageData(void *pReaderState)
{
    if (pReaderState != NULL)
        delete ((SHttpStageData*)pReaderState);
}

//! write a body part out
bool SHttpWriterStage::SendEvent_WriteBodyPart(SConnection *pConnection, SBodyPart *pBodyPart)
{
    return QueueEvent(SEvent(EVT_WRITE_BODY_PART, pConnection, pBodyPart));
}

//! Re orders and sends out http body parts to the socket
void SHttpWriterStage::HandleEvent(const SEvent &event)
{
    SConnection *   pConnection = (SConnection *)(event.pSource);
    SHttpStageData *pStageData  = (SHttpStageData *)pConnection->GetStageData(this);
    SBodyPart *     pBodyPart   = (SBodyPart *)(event.pData);
    int             result      = 0;

    if (event.evType == EVT_WRITE_DATA)
    {
    }
    else if (event.evType == EVT_WRITE_BODY_PART)
    {
        pStageData->SetRequest(pBodyPart->ExtraData<SHttpRequest *>());

        // first send the headers regardless of whether there are any 
        // body parts so it is done with
        if (pStageData->nextBPToSend == 0)
        {
            SHttpRequest *  pRequest    = pStageData->Request();
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
            }

            /*
            outStream << pResponse->Version() << " "
                      << pResponse->StatusCode() << " "
                      << pResponse->StatusMessage() << HttpUtils::CRLF;
            respHeaders.WriteHeaders(outStream);
            */
        }

        if (result >= 0 || pBodyPart != NULL)
        {
            pBodyPart               = pStageData->PutAndGetBodyPart(pBodyPart);
            while (pBodyPart != NULL)
            {
                result = WriteBodyPart(pConnection, pStageData, pBodyPart);
                if (result >= 0)
                {
                    pBodyPart = pStageData->NextBodyPart();
                }
                else
                {
                    pBodyPart = NULL;
                }
            }
        }

        if (result < 0)
        {
            SLogger::Get()->Log("TRACE: send error: [%d], EPIPE/ECONNRESET = [%d]/[%d] - [%s]\n", errno, EPIPE, ECONNRESET, strerror(errno));
            if (errno == EPIPE || errno == ECONNRESET)
            {
                pConnection->Server()->MarkConnectionAsClosed(pConnection);
                // shutdown(sockHandle, SHUT_WR);
                // assert("send error" && false);
            }
            else
            {
                assert("Some other error" && false);
            }
        }
    }
}

int SHttpWriterStage::WriteBodyPart(SConnection *     pConnection,
                                     SHttpStageData *  pStageData,
                                     SBodyPart *       pBodyPart)
{
    int             result      = 0;
    SHttpRequest *  pRequest    = pStageData->Request();
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
        pStageData->nextBP          = 0;
        pStageData->nextBPToSend    = 0;

        // remove and destroy the request from the queue
        // Note this destroys pRequest - 
        // dont use pRequest or Request() after this
        pStageData->DestroyRequest();

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
            pConnection->Server()->MarkConnectionAsClosed(pConnection);
        }
        else
        {
            // tell the reader we are ready for more
            // should we? or should we let the server take care of this?
            pConnection->SetState(SConnection::STATE_FINISHED);
            pReaderStage->SendEvent_ReadRequest(pConnection);
        }
    }
    else // treat as normal message
    {
        result = pBodyPart->WriteBodyToFD(pConnection->Socket());
        pStageData->nextBPToSend++;
    }

    // now delete the body part - its no longer needed!
    delete pBodyPart;
    return result;
}

