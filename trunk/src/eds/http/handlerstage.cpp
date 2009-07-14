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
    pRootModule(NULL)
{
}

//! Handle a new request - will be called by external request reader
bool SHttpHandlerStage::SendEvent_HandleRequest(SConnection *pConnection, SHttpRequest *pRequest)
{
    if (pRequest)
    {
        return QueueEvent(SEvent(EVT_REQUEST_ARRIVED, pConnection, pRequest));
    }
    else
    {
        pConnection->SetState(SConnection::STATE_READING);
        // tell the reader we are ready for more
        return pReaderStage->SendEvent_ReadRequest(pConnection);
    }
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
bool SHttpHandlerStage::SendEvent_OutputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart)
{
    if (pBodyPart)
    {
        pBodyPart->extra_data = pModule;
        return QueueEvent(SEvent(EVT_OUTPUT_BODY_TO_MODULE, pConnection, pBodyPart));
    }
    else
    {
        return QueueEvent(SEvent(EVT_NEXT_OUTPUT_MODULE, pConnection, pModule));
    }
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

