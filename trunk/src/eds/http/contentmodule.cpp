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
            pBodyPart->SetBody("--", 2);
            pBodyPart->AppendToBody(boundary);
            pBodyPart->AppendToBody("--", 2);
            pBodyPart->AppendToBody(CRLF, 2);

            // send to next module
            SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
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

        if (false)
        {
            pBodyPart->bpType = SBodyPart::BP_NORMAL;  // convert to normal message
            pBodyPart->SetBody("");

            while ( ! pModData->boundaries.empty())
            {
                SString boundary(pModData->boundaries.front());
                pModData->boundaries.pop_front();

                pBodyPart->AppendToBody("--", 2);
                pBodyPart->AppendToBody(boundary);
                pBodyPart->AppendToBody("--", 2);
                pBodyPart->AppendToBody(CRLF, 2);
            }
        }

        // send to next module so it can close it - nothing to do here
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
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
        }
        else
        {
            // prepend the 'current' boundary and send
            assert("No boundaries found in multi part message" && !pModData->boundaries.empty());

            SBodyPart *pBoundary = pResponse->NewBodyPart();
            SString boundary = pModData->boundaries.front();
            pBoundary->SetBody("--", 2);
            pBoundary->AppendToBody(boundary);
            pBoundary->AppendToBody(CRLF, 2);
            SendBodyPartToModule(pConnection, pStage, pBoundary, pModData, pNextModule);
        }

        std::cerr << "  === ContentModule:HandleBodyPart - " << "Sending Body Part to Next Module" << std::endl;
        SendBodyPartToModule(pConnection, pStage, pBodyPart, pModData, pNextModule);
    }
}

