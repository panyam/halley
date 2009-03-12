//*****************************************************************************
/*!
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
    std::string transferEncoding    = respHeaders.Header("Transfer-Encoding");

    pBodyPart->bpIndex = pModData->lastBPSent++;
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
            std::stringstream sizestr;
            sizestr << hex << size << CRLF;
            pBodyPart->PrependToBody(sizestr.str());
            pBodyPart->AppendToBody(CRLF, 2);
        }
    }

    pStage->OutputToModule(pConnection, pNextModule, pBodyPart);
}

