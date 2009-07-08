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
 *  \file   writermodule.cpp
 *
 *  \brief  A module that writes the request to the network - this MUST be
 *  the last stage in a chain.
 *
 *  \version
 *      - S Panyam      11/03/2009
 *        Created
 *
 *****************************************************************************/

#include "writermodule.h"
#include "eds/connection.h"
#include "handlerstage.h"
#include "request.h"
#include "response.h"

//! This affects transfer-xxx headers but not content headers
//
// Also this only takes place if 
void SWriterModule::ProcessOutput(SHttpHandlerData *    pHandlerData,
                                  SHttpHandlerStage *   pStage,
                                  SBodyPart *           pBodyPart)
{
    SHttpModuleData *pModData   = pHandlerData->GetModuleData(this, true);
    std::ostream &outStream(pHandlerData->pConnection->GetOutputStream());

    // first send the headers regardless of whether there are any 
    // body parts so it is done with
    if (pModData->nextBPToSend == 0)
    {
        SHttpRequest *  pRequest    = pHandlerData->Request();
        SHttpResponse * pResponse   = pRequest->Response();
        SHeaderTable &  respHeaders = pResponse->Headers();
        // write headers
        SString transferEncoding(respHeaders.Header("Transfer-Encoding"));
        if (strcasecmp(transferEncoding.c_str(), "chunked") == 0)
        {
            respHeaders.RemoveHeader("Content-Length");
        }
        respHeaders.Lock();
        std::cerr << "  === WriterModule - " << "Writing Message Headers" << std::endl;
        outStream << pResponse->Version() << " "
                  << pResponse->StatusCode() << " "
                  << pResponse->StatusMessage() << HttpUtils::CRLF;
        respHeaders.WriteHeaders(outStream);
    }

    if (pBodyPart != NULL)
    {
        pBodyPart               = pModData->PutAndGetBodyPart(pBodyPart);
        while (pBodyPart != NULL)
        {
            if (HandleBodyPart(pHandlerData, pStage, pModData, pBodyPart, outStream))
            {
                pBodyPart = pModData->NextBodyPart();
            }
            else
            {
                pBodyPart = NULL;
            }
        }
    }
}


bool SWriterModule::HandleBodyPart(SHttpHandlerData *   pHandlerData, 
                                   SHttpHandlerStage *  pStage,
                                   SHttpModuleData *    pModData,
                                   SBodyPart *          pBodyPart, 
                                   std::ostream &       outStream)
{
    SConnection *pConnection        = pHandlerData->pConnection;
    SHttpRequest *  pRequest        = pHandlerData->Request();
    SHttpResponse * pResponse       = pRequest->Response();
    // SHeaderTable &reqHeaders        = pRequest->Headers();
    SHeaderTable &respHeaders       = pResponse->Headers();
    SString transferEncoding(respHeaders.Header("Transfer-Encoding"));

    if (pBodyPart->Type() == SBodyPart::BP_CONTENT_FINISHED ||
        pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION)
    {
        // reset last BP sent as no more packets will 
        // be sent for this request
        pModData->nextBP        = 0;
        pModData->nextBPToSend  = 0;

        // do nothing - close connection only if close header found
        if (pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION ||
            pRequest->Headers().CloseConnection())
        {
            std::cerr << "  === WriterModule - " << "Closing Connection" << std::endl;
            pStage->CloseConnection(pConnection);
            return false;
        }
        else
        {
            std::cerr << "  === WriterModule - " << "Destroying Request" << std::endl;
            // remove and destroy the request from the queue
            // Note this destroys pRequest - dont use pRequest 
            // after this
            pHandlerData->DestroyRequest();

            // Trigger the handling of the next request
            pStage->HandleRequest(pConnection, NULL);
        }
    }
    else // treat as normal message
    {
        std::cerr << "  === WriterModule - " << "Writing Message Body" << std::endl;
        pBodyPart->WriteMessageBody(outStream);
        pModData->nextBPToSend++;
    }

    // now delete the body part - its no longer needed!
    delete pBodyPart;
    return true;
}

