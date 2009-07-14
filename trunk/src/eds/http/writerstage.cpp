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

//! Re orders and sends out http body parts to the socket
void SHttpWriterStage::HandleEvent(const SEvent &event)
{
    SConnection *       pConnection = (SConnection *)(event.pSource);
    SHttpStageData *    pStageData  = (SHttpStageData *)pConnection->GetStageData(this);
    SBodyPart *         pBodyPart   = (SBodyPart *)(event.pData);
    std::ostream &outStream(pConnection->GetOutputStream());

    if (event.evType == EVT_WRITE_DATA)
    {
    }
    else if (event.evType == EVT_WRITE_BODY_PART)
    {
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
            outStream << pResponse->Version() << " "
                      << pResponse->StatusCode() << " "
                      << pResponse->StatusMessage() << HttpUtils::CRLF;
            respHeaders.WriteHeaders(outStream);
        }

        if (pBodyPart != NULL)
        {
            pBodyPart               = pStageData->PutAndGetBodyPart(pBodyPart);
            while (pBodyPart != NULL)
            {
                if (HandleBodyPart(pConnection, pStageData, pBodyPart, outStream))
                {
                    pBodyPart = pStageData->NextBodyPart();
                }
                else
                {
                    pBodyPart = NULL;
                }
            }
        }
    }
}

bool SHttpWriterStage::HandleBodyPart(SConnection *     pConnection,
                                  SHttpStageData *  pStageData,
                                  SBodyPart *       pBodyPart,
                                  std::ostream &    outStream)
{
    SHttpRequest *  pRequest        = pStageData->Request();
    SHttpResponse * pResponse       = pRequest->Response();
    // SHeaderTable &reqHeaders        = pRequest->Headers();
    SHeaderTable &respHeaders       = pResponse->Headers();
    SString transferEncoding(respHeaders.Header("Transfer-Encoding"));

    if (pBodyPart->Type() == SBodyPart::BP_CONTENT_FINISHED ||
        pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION)
    {
        // reset last BP sent as no more packets will 
        // be sent for this request
        pStageData->nextBP        = 0;
        pStageData->nextBPToSend  = 0;

        // do nothing - close connection only if close header found
        if (pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION ||
            pRequest->Headers().CloseConnection())
        {
            // remove and destroy the request from the queue
            // Note this destroys pRequest - 
            // dont use pRequest or Request() after this
            pStageData->DestroyRequest();

            // a connection is to be closed -
            // the problem is regardless of how many threads we or other stages
            // have, killing it here will pose sever problems.  So instead of
            // killing, we flag it as being closed so nothing else uses this
            // connection
            pConnection->Server()->MarkConnectionAsClosed(pConnection);
        }
        else
        {
            // remove and destroy the request from the queue
            // Note this destroys pRequest - 
            // dont use pRequest or Request() after this
            pStageData->DestroyRequest();
        }
    }
    else // treat as normal message
    {
        pBodyPart->WriteMessageBody(outStream);
        pStageData->nextBPToSend++;
    }

    // now delete the body part - its no longer needed!
    delete pBodyPart;
    return true;
}

