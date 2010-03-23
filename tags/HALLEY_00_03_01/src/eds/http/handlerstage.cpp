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
 *  \file   handlerstage.cpp
 *
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#include "request.h"
#include "response.h"
#include "eds/connection.h"
#include "eds/server.h"
#include "eds/stage.h"
#include "eds/connection.h"
#include "utils/membuff.h"
#include "readerstage.h"
#include "writerstage.h"
#include "handlerstage.h"
#include "httpmodule.h"

#include <iostream>
using std::cerr;
using std::endl;

// Creates a handler stage
SHttpHandlerStage::SHttpHandlerStage(const SString &name, int numThreads)
:
    SStage(name, numThreads),
    pReaderStage(NULL),
    pWriterStage(NULL),
    pRootModule(NULL)
{
}

//! Sends input to be processed by a module
bool SHttpHandlerStage::SendEvent_InputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart)
{
    if (pBodyPart)
    {
        pBodyPart->extra_data = pModule;
        return QueueEvent(SEvent(EVT_INPUT_BODY_TO_MODULE, pConnection, pBodyPart));
    }
    else
    {
        return QueueEvent(SEvent(EVT_NEXT_INPUT_MODULE, pConnection, pModule));
    }
}

//! Sends output to be processed by a module
bool SHttpHandlerStage::SendEvent_OutputToModule(SConnection *  pConnection,
                                                 SHttpModule *  pNextModule,
                                                 SBodyPart *    pBodyPart)
{
    assert("Next module CANNOT be NULL" && pNextModule != NULL);
    if (pBodyPart)
    {
        pBodyPart->extra_data = pNextModule;
        return QueueEvent(SEvent(EVT_OUTPUT_BODY_TO_MODULE, pConnection, pBodyPart));
    }
    else
    {
        return QueueEvent(SEvent(EVT_NEXT_OUTPUT_MODULE, pConnection, pNextModule));
    }
}

//! Handle a new request - MUST be called by external request reader
bool SHttpHandlerStage::SendEvent_HandleNextRequest(SConnection *pConnection, SHttpRequest *pRequest)
{
    assert("Request CANNOT be NULL" && pRequest != NULL);
    return QueueEvent(SEvent(EVT_REQUEST_ARRIVED, pConnection, pRequest));
}

//! Sends body part to be written out - MUST be called by a module
bool SHttpHandlerStage::SendEvent_WriteBodyPart(SConnection *   pConnection,
                                                SHttpRequest *  pRequest,
                                                SBodyPart *     pBodyPart)
{
    // otherwise get the writer stage to send it out
    assert("Request AND BodyPart must be NON-NULL" && pRequest != NULL && pBodyPart != NULL);
    pBodyPart->extra_data = pRequest;
    return pWriterStage->SendEvent_WriteBodyPart(pConnection, pBodyPart);
}

void SHttpHandlerStage::JobDestroyed(SJob *pJob)
{
    if (pJob != NULL)
    {
        SHttpHandlerData *pStageData = (SHttpHandlerData *)pJob->GetStageData(this);
        if (pStageData != NULL)
        {
            delete pStageData;
            pJob->SetStageData(this, NULL);
        }
    }
}

//! Handles a request after it has been read.
//
// Will call the RequestHandler stage when a complete request 
// has been read.
void SHttpHandlerStage::HandleEvent(const SEvent &event)
{
    // The connection currently being processed
    SConnection *       pConnection     = (SConnection *)(event.pSource);
    SHttpHandlerData *  pHandlerData    = (SHttpHandlerData *)pConnection->GetStageData(this);

    // create the response if none yet - it MUST be an "arrived" event
    if (pHandlerData == NULL)
    {
        pHandlerData = new SHttpHandlerData(pConnection);
        pConnection->SetStageData(this, pHandlerData);
        pConnection->AddListener(this);
    }

    SBodyPart *pBodyPart    = NULL;
    SHttpRequest *pRequest  = NULL;
    switch (event.evType)
    {
        case EVT_REQUEST_ARRIVED:
            // new request - so reset all module data for the request
            pHandlerData->ResetModuleData();
            pRequest = event.Data<SHttpRequest *>();
            pHandlerData->SetRequest(pRequest);

            // let the root module take it if there is a valid request
            if (pRequest)
                pRootModule->ProcessInput(pHandlerData, this, NULL);
            break ;
        case EVT_INPUT_BODY_TO_MODULE:   // let the next module handle input
            pBodyPart = event.Data<SBodyPart *>();
            pBodyPart->ExtraData<SHttpModule *>()->ProcessInput(pHandlerData, this, pBodyPart);
            break ;
        case EVT_NEXT_INPUT_MODULE:
            event.Data<SHttpModule *>()->ProcessInput(pHandlerData, this, NULL);
            break ;
        case EVT_OUTPUT_BODY_TO_MODULE:   // let the next module handle input
            pBodyPart = event.Data<SBodyPart*>();
            pBodyPart->ExtraData<SHttpModule *>()->ProcessOutput(pHandlerData, this, pBodyPart);
            break ;
        case EVT_NEXT_OUTPUT_MODULE:
            event.Data<SHttpModule *>()->ProcessOutput(pHandlerData, this, NULL);
            break ;
    }
}
