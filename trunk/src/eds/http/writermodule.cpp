//*****************************************************************************
/*!
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

    // already being processed quit
    // TODO: not yet thread safe - use "trylock"ed mutexes for this
    if (pModData->processing)
        return ;

    std::ostream &outStream(pHandlerData->pConnection->GetOutputStream());
    pModData->processing    = true;

    // first send the headers regardless of whether there are any 
    // body parts so it is done with
    if (pModData->lastBPSent == 0)
    {
        SHttpRequest *  pRequest    = pHandlerData->Request();
        SHttpResponse * pResponse   = pRequest->Response();
        SHeaderTable &  respHeaders = pResponse->Headers();
        // write headers
        std::string transferEncoding(respHeaders.Header("Transfer-Encoding"));
        if (transferEncoding == "chunked")
        {
            respHeaders.RemoveHeader("Content-Length");
        }
        respHeaders.Lock();
        std::cerr << "  === WriterModule - " << "Writing Message Headers" << std::endl;
        outStream << pResponse->Version() << " "
                  << pResponse->StatusCode() << " "
                  << pResponse->StatusMessage() << CRLF;
        respHeaders.WriteHeaders(outStream);
        outStream.flush();
    }

    if (pBodyPart != NULL)
    {
        pBodyPart               = pModData->PutAndGetBodyPart(pBodyPart);
        while (pBodyPart != NULL)
        {
            HandleBodyPart(pHandlerData, pStage, pModData, pBodyPart, outStream);

            pBodyPart = pModData->NextBodyPart();
        }
    }

    // turn off processing flag so it can be resumed in the future
    pModData->processing    = false;
}


void SWriterModule::HandleBodyPart(SHttpHandlerData *   pHandlerData, 
                                   SHttpHandlerStage *  pStage,
                                   SHttpModuleData *    pModData,
                                   SBodyPart *          pBodyPart, 
                                   std::ostream &       outStream)
{
    SConnection *pConnection        = pHandlerData->pConnection;
    SHttpRequest *  pRequest        = pHandlerData->Request();
    SHttpResponse * pResponse       = pRequest->Response();
    SHeaderTable &respHeaders       = pResponse->Headers();
    std::string transferEncoding(respHeaders.Header("Transfer-Encoding"));

    if (pBodyPart->Type() == SBodyPart::BP_CLOSE_CONNECTION)
    {
        // TODO: how to ignore all future body parts since connection is
        // closed and/or finished?
        pStage->CloseConnection(pConnection);
    }
    else if (pBodyPart->Type() == SBodyPart::BP_CONTENT_FINISHED)
    {
        // do nothing - close connection only if close header found
        std::string closehdr;
        respHeaders.HeaderIfExists("Connection", closehdr);
        if (strcasecmp(closehdr.c_str(), "close") == 0)
        {
            pStage->CloseConnection(pConnection);
        }
        else
        {
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
        outStream.flush();
    }

    // now delete the body part - its no longer needed!
    delete pBodyPart;

    pModData->lastBPSent++;
}

