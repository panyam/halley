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
 *  \file   httpmodule.cpp
 *
 *  \brief  Handler modules used by handler stages to handler requests.
 *
 *  \version
 *      - S Panyam      04/03/2009
 *        Created
 *
 *****************************************************************************/

#include "request.h"
#include "response.h"
#include "eds/connection.h"
#include "eds/server.h"
#include "eds/http/handlerstage.h"
#include "eds/connection.h"
#include "utils/membuff.h"
#include "httpmodule.h"

#include <iostream>
using std::cerr;
using std::endl;

//! Compares 2 body parts based on their indices
bool SHttpModuleData::SBPComparer::operator()(const SBodyPart *a, const SBodyPart *b) const
{
    return b->bpIndex < a->bpIndex;
}

//! Destroys module data
SHttpModuleData::~SHttpModuleData()
{
    // TODO: destroy the body parts!
    while ( ! bodyParts.empty())
    {
        SBodyPart *part = bodyParts.top();
        bodyParts.pop();
        delete part;
    }
}

//! Returns the next body part if it matches the nextBP count
SBodyPart *SHttpModuleData::NextBodyPart()
{
    SBodyPart *pOut = NULL;

    if (!bodyParts.empty())
    {
        SBodyPart *pTop = bodyParts.top();
        if (pTop->Index() == nextBP)
        {
            nextBP++;
            pOut = pTop;
            bodyParts.pop();
        }
    }

    return pOut;
}

//! Gets the next body part that can be processed (if any).
void SHttpModuleData::PutBodyPart(SBodyPart *pCurrPart)
{
    if (pCurrPart != NULL)
        bodyParts.push(pCurrPart);
}

//! Short cut for put and next calls
SBodyPart *SHttpModuleData::PutAndGetBodyPart(SBodyPart *pCurrPart)
{
    PutBodyPart(pCurrPart);
    return NextBodyPart();
}

//! Create a state object
SHttpHandlerData::SHttpHandlerData(SConnection *pConn) :   pConnection(pConn)
{
}

//! Destroy the state
SHttpHandlerData::~SHttpHandlerData()
{
    // TODO: should we delete requests?
    // delete pRequest;

    // remove all module specific data
    std::list<ModuleData *>::iterator lastModule = moduleData.end();
    for (std::list<ModuleData *>::iterator iter = moduleData.begin();
            iter != lastModule; ++iter)
    {
        ModuleData *pModData = *iter;
        pModData->first->RemoveModuleData(pModData->second);
        delete pModData;
    }
    moduleData.clear();
}

//! Resets all modules when a new request begins.
void SHttpHandlerData::ResetModuleData()
{
    // reset all module specific data - we could erase like in the
    // destructor but that may not be necessary for sake of efficiency

    std::list<ModuleData *>::iterator lastModule = moduleData.end();
    for (std::list<ModuleData *>::iterator iter = moduleData.begin();
            iter != lastModule; ++iter)
    {
        ModuleData *pModData = *iter;
        pModData->second->Reset();
    }
}

//! Destroys the current request
void SHttpHandlerData::DestroyRequest()
{
    if ( ! requests.empty())
    {
        SHttpRequest *pRequest = requests.front();
        requests.pop_front();
        delete pRequest;
    }
}

//! Sets the module specific data - so each module can keep its own
// state regarding this connection
void SHttpHandlerData::SetModuleData(SHttpModule *pModule, SHttpModuleData *pData)
{
    std::list<ModuleData *>::iterator lastModule = moduleData.end();
    for (std::list<ModuleData *>::iterator iter = moduleData.begin();
            iter != lastModule;
            ++iter)
    {
        ModuleData *pModData = *iter;
        if (pModData->first == pModule && pModData->second != pData)
        {
            // remove the old data
            pModule->RemoveModuleData(pModData->second);

            // set new data and return
            pModData->second = pData;
            return ;
        }
    }
    moduleData.push_back(new ModuleData(pModule, pData));
}

//! Sets the module specific data - so each module can keep its own
// state regarding this connection
SHttpModuleData *SHttpHandlerData::GetModuleData(SHttpModule *pModule, bool create)
{
    std::list<ModuleData *>::iterator lastModule = moduleData.end();
    for (std::list<ModuleData *>::iterator iter = moduleData.begin();
            iter != lastModule;
            ++iter)
    {
        ModuleData *pModData = *iter;
        if (pModData->first == pModule)
            return pModData->second;
    }

    SHttpModuleData *pModData = NULL;
    if (create)
    {
        pModData = pModule->CreateModuleData(this);
        pModData->Reset();
        moduleData.push_back(new ModuleData(pModule, pModData));
    }
    return pModData;
}


//! Creates new module data if necessary
SHttpModuleData *SHttpModule::CreateModuleData(SHttpHandlerData *pHandlerData)
{
    return new SHttpModuleData();
}

//! Send a body part to another module.
void SHttpModule::SendBodyPartToModule(SConnection *        pConnection,
                                       SHttpHandlerStage *  pStage,
                                       SBodyPart *          pBodyPart,
                                       SHttpModuleData *    pModData,
                                       SHttpModule *        pModule)
{
    if (pBodyPart != NULL)
        pBodyPart->bpIndex = pModData->nextBPToSend++;
    pStage->SendEvent_OutputToModule(pConnection, pNextModule, pBodyPart);
}

