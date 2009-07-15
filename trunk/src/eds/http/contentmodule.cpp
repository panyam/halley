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
 *  \file   contentModule.cpp
 *
 *  \brief  Module for content coding all output.
 *
 *  \version
 *      - S Panyam      10/03/2009
 *        Created
 *
 *****************************************************************************/

#include "eds/connection.h"
#include "contentmodule.h"
#include "handlerstage.h"
#include "request.h"
#include "response.h"

void SContentModule::ProcessOutput(SConnection *        pConnection,
                                   SHttpHandlerData *   pHandlerData,
                                   SHttpHandlerStage *  pStage,
                                   SBodyPart *          pBodyPart)
{
    SContentModuleData *pModData    = dynamic_cast<SContentModuleData *>(pHandlerData->GetModuleData(this, true));

    // already being processed quit
    // TODO: not yet thread safe
    if (pModData->IsProcessing())
    {
        assert("Already processing..." && false);
        return ;
    }

    pModData->SetProcessing(true);

    if (pBodyPart != NULL) // we have multi part messages
    {
        pBodyPart               = pModData->PutAndGetBodyPart(pBodyPart);

        while (pBodyPart != NULL)
        {
            HandleBodyPart(pConnection, pHandlerData, pStage, pModData, pBodyPart);

            pBodyPart = pModData->NextBodyPart();
        }
    }
    else
    {
        SendBodyPartToModule(pConnection, pStage, pHandlerData->Request(), pBodyPart, pModData, pNextModule);
    }

    // turn off processing flag so it can be resumed in the future
    pModData->SetProcessing(false);
}

void SContentModule::HandleBodyPart(SConnection *       pConnection,
                                    SHttpHandlerData *  pHandlerData, 
                                    SHttpHandlerStage * pStage,
                                    SContentModuleData *pModData,
                                    SBodyPart *         pBodyPart)
{
    SHttpRequest *  pRequest    = pHandlerData->Request();
    SHttpResponse * pResponse   = pRequest->Response();
    int bpType                  = pBodyPart->Type();

    if (bpType == SBodyPart::BP_OPEN_SUB_MESSAGE)
    {
        // cannot send an open sub message command 
        // if we dont have multi part messages
        assert ("Content-Type MUST be multipart" && pResponse->IsMultipart());

        SString boundary(pBodyPart->data.begin(), pBodyPart->data.end());
        pModData->boundaries.push_front(boundary);

        // done with the body part so delete it!
        delete pBodyPart;
    }
    else if (bpType == SBodyPart::BP_CLOSE_SUB_MESSAGE)
    {
        // cannot send an open sub message command 
        // if we dont have multi part messages
        assert ("Content-Type MUST be multipart" && pResponse->IsMultipart());

        if (!pModData->boundaries.empty())
        {
            SString boundary(pModData->boundaries.front());
            pModData->boundaries.pop_front();
            pBodyPart->bpType = SBodyPart::BP_NORMAL;  // convert to normal message
            pBodyPart->SetBody(HttpUtils::CRLF, 2);
            pBodyPart->AppendToBody("--", 2);
            pBodyPart->AppendToBody(boundary);
            pBodyPart->AppendToBody("--", 2);

            // send to next module
            SendBodyPartToModule(pConnection, pStage, pRequest, pBodyPart, pModData, pNextModule);
        }
        else
        {
            // no boundaries so cant send this
            delete pBodyPart;
        }
    }
    else if (bpType == SBodyPart::BP_CLOSE_CONNECTION || bpType == SBodyPart::BP_CONTENT_FINISHED)
    {
        // Send a whole bunch of Close boundary calls!!

        if (!pModData->boundaries.empty())
        {
            SBodyPart *pCloser = pResponse->NewBodyPart();

            while ( ! pModData->boundaries.empty())
            {
                SString boundary(pModData->boundaries.front());
                pModData->boundaries.pop_front();

                pCloser->AppendToBody(HttpUtils::CRLF, 2);
                pCloser->AppendToBody("--", 2);
                pCloser->AppendToBody(boundary);
                pCloser->AppendToBody("--", 2);
            }

            SendBodyPartToModule(pConnection, pStage, pRequest, pCloser, pModData, pNextModule);
        }

        // send to next module so it can close it - nothing to do here
        SendBodyPartToModule(pConnection, pStage, pRequest, pBodyPart, pModData, pNextModule);
    }
    else if (pBodyPart)
    {
        // we have normal messages - so update the contentLength if
        // necessary and send to next module
        if (!pResponse->IsMultipart())
        {
            SHeaderTable &  respHeaders = pResponse->Headers();
            // append to body (can ignore sub messages as it is single part)
            int bodySize    = pBodyPart->Size();
            if (bodySize > 0)
            {
                int contLength = -1;
                SString hdrContLength;
                if (respHeaders.HeaderIfExists("Content-Length", hdrContLength))
                {
                    contLength  = atoi(hdrContLength.c_str());
                }

                // For now set the contlength header to be what ever
                // the body size is and not worry about caching.  The
                // application or lower layers MUST do the caching.
                if (bodySize != contLength)
                {
                    respHeaders.SetIntHeader("Content-Length", bodySize);
                }
            }

            SendBodyPartToModule(pConnection, pStage, pRequest, pBodyPart, pModData, pNextModule);
        }
        else
        {
            // prepend the 'current' boundary and send
            assert("No boundaries found in multi part message" && !pModData->boundaries.empty());

            SStringStream boundary;
            boundary << HttpUtils::CRLF << "--" << pModData->boundaries.front() << HttpUtils::CRLF;
            // boundary << "Content-Type: " << "text/text" << HttpUtils::CRLF;
            boundary << "Content-Length: " << pBodyPart->Size() << HttpUtils::CRLF << HttpUtils::CRLF;

            pBodyPart->InsertInBody(boundary.str());

            SendBodyPartToModule(pConnection, pStage, pRequest, pBodyPart, pModData, pNextModule);
        }
    }
}

