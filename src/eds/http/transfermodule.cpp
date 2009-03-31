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
 *  \file   transfermodule.cpp
 *
 *  \brief  Module for transfer coding all output.
 *
 *  \version
 *      - S Panyam      10/03/2009
 *        Created
 *
 *****************************************************************************/

#include "transfermodule.h"
#include "eds/connection.h"
#include "handlerstage.h"
#include "request.h"
#include "response.h"

//! This affects transfer-xxx headers but not content headers
//
// Also this only takes place if 
void STransferModule::ProcessOutput(SHttpHandlerData *  pHandlerData,
                                    SHttpHandlerStage * pStage,
                                    SBodyPart *         pBodyPart)
{
    SHttpModuleData *pModData   = pHandlerData->GetModuleData(this, true);

    // already being processed quit
    // TODO: not yet thread safe
    if (pModData->processing)
        return ;

    pModData->processing    = true;

    // how do we handle empty bodies?
    if (pBodyPart != NULL)
    {
        pBodyPart               = pModData->PutAndGetBodyPart(pBodyPart);
        while (pBodyPart != NULL)
        {
            HandleBodyPart(pHandlerData, pStage, pModData, pBodyPart);

            pBodyPart = pModData->NextBodyPart();
        }
    }
    else
    {
        // otherwise send it as is to the next module, cant just ignore a
        // request to process output!
        SendBodyPartToModule(pHandlerData->pConnection, pStage, pBodyPart, pModData, pNextModule);
    }

    // turn off processing flag so it can be resumed in the future
    pModData->processing    = false;
}


void STransferModule::HandleBodyPart(SHttpHandlerData *   pHandlerData, 
                                   SHttpHandlerStage *  pStage,
                                   SHttpModuleData *    pModData,
                                   SBodyPart *          pBodyPart)
{
    SConnection *pConnection    = pHandlerData->pConnection;
    SHttpRequest *pRequest      = pHandlerData->Request();
    SHttpResponse *pResponse    = pRequest->Response();
    SHeaderTable &respHeaders   = pResponse->Headers();
    SString transferEncoding    = respHeaders.Header("Transfer-Encoding");

    pBodyPart->bpIndex = pModData->nextBPToSend++;
    if (pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION ||
             pBodyPart->Type() == SBodyPart::BP_CONTENT_FINISHED)
    {
        SendBodyPartToModule(pHandlerData->pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
    else
    {
        // write size first then the content
        unsigned size = pBodyPart->Size();
        if (transferEncoding == "chunked")
        {
            SStringStream sizestr;
            sizestr << hex << size << CRLF;
            pBodyPart->InsertInBody(sizestr.str(), 0);
            pBodyPart->AppendToBody(CRLF, 2);
        }
    }

    pStage->OutputToModule(pConnection, pNextModule, pBodyPart);
}

