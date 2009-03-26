//*****************************************************************************
/*!
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
#include "eds/fileiohelper.h"
#include "eds/dbhelperstage.h"
#include "eds/connection.h"
#include "eds/server.h"
#include "eds/stage.h"
#include "eds/connection.h"
#include "utils/membuff.h"
#include "handlerstage.h"
#include "httpmodule.h"

#include <iostream>
using std::cerr;
using std::endl;

// Creates a new file io helper stage
SHttpHandlerStage::SHttpHandlerStage(int numThreads)
:
    SStage(numThreads),
    pIOHelper(NULL),
    pDBHelper(NULL),
    pRootModule(NULL)
{
}


// Set the File IO Helper stage
void SHttpHandlerStage::SetIOHelper(SFileIOHelper *pHelper)
{
    pIOHelper = pHelper;
}

// Set the DB helper stage
void SHttpHandlerStage::SetDBHelper(SDBHelperStage *pHelper)
{
    pDBHelper = pHelper;
}

//! Handle a new request - will be called by external request reader
void SHttpHandlerStage::HandleRequest(SConnection *pConnection, SHttpRequest *pRequest)
{
    QueueEvent(SEvent(EVT_REQUEST_ARRIVED, pConnection, pRequest));
}
    
//! Sends input to be processed by a module
void SHttpHandlerStage::InputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart)
{
    if (pBodyPart)
    {
        pBodyPart->extra_data = pModule;
        QueueEvent(SEvent(EVT_INPUT_BODY_TO_MODULE, pConnection, pBodyPart));
    }
    else
    {
        QueueEvent(SEvent(EVT_NEXT_INPUT_MODULE, pConnection, pModule));
    }
}

//! Sends output to be processed by a module
void SHttpHandlerStage::OutputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart)
{
    if (pBodyPart)
    {
        pBodyPart->extra_data = pModule;
        QueueEvent(SEvent(EVT_OUTPUT_BODY_TO_MODULE, pConnection, pBodyPart));
    }
    else
    {
        QueueEvent(SEvent(EVT_NEXT_OUTPUT_MODULE, pConnection, pModule));
    }
}

//! Request to close the connection
void SHttpHandlerStage::CloseConnection(SConnection *pConnection)
{
    QueueEvent(SEvent(EVT_CLOSE_CONNECTION, pConnection));
}

//! Handles a request after it has been read.
//
// Will call the RequestHandler stage when a complete request 
// has been read.
void SHttpHandlerStage::HandleEvent(const SEvent &event)
{
    // The connection currently being processed
    SConnection *pConnection            = (SConnection *)(event.pSource);
    SHttpHandlerData *pHandlerData      = (SHttpHandlerData *)pConnection->GetStageData(this);

    // create the response if none yet - it MUST be an "arrived" event
    if (pHandlerData == NULL)
    {
        pHandlerData = new SHttpHandlerData(pConnection);
        pConnection->SetStageData(this, pHandlerData);
    }

    SBodyPart *pBodyPart    = NULL;
    SHttpRequest *pRequest  = NULL;
    switch (event.evType)
    {
        case EVT_REQUEST_ARRIVED:
            // new request - so reset all module data for the request
            pHandlerData->ResetModuleData();
            pRequest = event.Data<SHttpRequest *>();
            if (pRequest) pHandlerData->AddRequest(pRequest);

            // let the root module take it
            if (pHandlerData->Request())
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
        case EVT_CLOSE_CONNECTION: // close the connection
            pConnection->Close();
            pConnection->SetStageData(this, NULL);
            delete pHandlerData;
            pHandlerData = NULL;
            break ;
    }
}

