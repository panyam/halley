//*****************************************************************************
/*!
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

void SContentModule::ProcessOutput(SHttpHandlerData *   pHandlerData,
                                   SHttpHandlerStage *  pStage,
                                   SBodyPart *          pBodyPart)
{
    SContentModuleData *pModData    = (SContentModuleData *)(pHandlerData->GetModuleData(this, true));

    // already being processed quit
    // TODO: not yet thread safe
    if (pModData->processing)
    {
        std::cerr << "  === ContentModule:ProcessOutput - " << "Already procesing - Quitting." << pBodyPart << std::endl;
        return ;
    }

    pModData->processing    = true;

    if (pBodyPart != NULL) // we have multi part messages
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
        std::cerr << "  === ContentModule:ProcessOutput - " << "Sending Body Part to Next Module: " << pBodyPart << std::endl;
        SendBodyPartToModule(pHandlerData->pConnection, pStage, pBodyPart, pModData, pNextModule);
    }

    // turn off processing flag so it can be resumed in the future
    pModData->processing    = false;
}

void SContentModule::HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                                    SHttpHandlerStage * pStage,
                                    SContentModuleData *pModData,
                                    SBodyPart *         pBodyPart)
{
    SConnection *pConnection    = pHandlerData->pConnection;
    int bpType                  = pBodyPart->Type();
    std::string boundary;

    if (bpType == SBodyPart::BP_OPEN_SUB_MESSAGE)
    {
        boundary = std::string(pBodyPart->data.begin(), pBodyPart->data.end());
        pModData->boundaries.push_front(boundary);
        pBodyPart->bpType = SBodyPart::BP_NORMAL;  // convert to normal message
        pBodyPart->SetBody("--", 2);
        pBodyPart->AppendToBody(boundary);
        pBodyPart->AppendToBody(CRLF, 2);

        // send to next module
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
    else if (bpType == SBodyPart::BP_CLOSE_SUB_MESSAGE && !pModData->boundaries.empty())
    {
        boundary = pModData->boundaries.front();
        pModData->boundaries.pop_front();
        pBodyPart->bpType = SBodyPart::BP_NORMAL;  // convert to normal message
        pBodyPart->SetBody("--", 2);
        pBodyPart->AppendToBody(boundary);
        pBodyPart->AppendToBody("--", 2);
        pBodyPart->AppendToBody(CRLF, 2);

        // send to next module
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
    else if (bpType == SBodyPart::BP_CLOSE_CONNECTION || bpType == SBodyPart::BP_CONTENT_FINISHED)
    {
        // send to next module so it can close it - nothing to do here
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
    else    // treat a normal message
    {
        // we have normal messages - so update the contentLength if
        // necessary and send to next module
        SHttpRequest *  pRequest    = pHandlerData->Request();
        SHttpResponse * pResponse   = pRequest->Response();
        if (!pResponse->IsMultipart())
        {
            SHeaderTable &  respHeaders = pResponse->Headers();
            // append body parts and keep adding to the content length
            if (pBodyPart)
            {
                // append to body (can ignore sub messages as it is single part)
                int bodySize    = pBodyPart->Size();
                if (bodySize > 0)
                {
                    int contLength = -1;
                    std::string hdrContLength;
                    if (respHeaders.HeaderIfExists("Content-Length", hdrContLength))
                    {
                        contLength  = atoi(hdrContLength.c_str());
                    }

                    // there are 2 issues here... who should be doing the
                    // caching?  Consider following scenarious:
                    //
                    // App sends cont-block1 to this module (block size =
                    // 10)
                    //
                    //      1. contLength is set
                    //      2. contLength is not set
                    //
                    // App now sends cont-block2 to this module (say with
                    // block size of 20)
                    //
                    // What should contLength of the response be?
                    //
                    // This is related to who should be doing the caching.
                    // If this module is doing the caching (for singlepart
                    // messages) then it can:
                    //
                    // set the contlength header only if it is missing (ie
                    // not explicitly sent by child modules)
                    //
                    // or it can cache all body parts for a single part
                    // message and calculate teh contentLength at the end
                    // after accumulating all body parts before being sent
                    // out to the next module
                    //
                    // For now do the former - set the contlength header to
                    // be what ever the body size is and not worry about
                    // caching.  The application or lower layers MUST do
                    // the caching.
                    if (bodySize != contLength)
                    {
                        respHeaders.SetIntHeader("Content-Length", bodySize);
                    }
                }
            }
        }

        // TODO: We have to check if the message is for a child message or
        // the main message - by inspecting the boundary stack.
        //
        // TODO: Also the messages must be cached instead of being sent out
        // if it is a single part message and content length is less than
        // body length
        std::cerr << "  === ContentModule:HandleBodyPart - " << "Sending Body Part to Next Module" << std::endl;
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
}

