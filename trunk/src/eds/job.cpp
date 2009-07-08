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
 *  \file   job.cpp
 *
 *  \brief
 *
 *  A job in an event driver system.
 *
 *  \version
 *      - Sri Panyam      20/02/2009
 *        Created
 *
 *****************************************************************************/

#include "job.h"
#include "stage.h"

/**************************************************************************************
*   \brief  Create a new job object
*
*   \version
*       - Sri Panyam  26/03/2009
*         Created
**************************************************************************************/
SJob::SJob()
{
    pOwner      = NULL;
    isAlive     = true;

    // create space for the first 5 stages
    for (int i = 0;i < 5;i++)
    {
        stageData.push_back(NULL);
    }
}

/**************************************************************************************
*   \brief  Destroys a job and clears the stage data
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
SJob::~SJob()
{
    isAlive = false;
    stageData.clear();
}

/**************************************************************************************
*   \brief  Adds a job listener.
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
bool SJob::AddListener(SJobListener *pListener)
{
    if (pListener != NULL)
    {
        std::list<SJobListener *>::iterator iter = find(listeners.begin(), listeners.end(), pListener);
        if (iter == listeners.end())
        {
            listeners.push_back(pListener);
            return true;
        }
    }
    return false;
}

/**************************************************************************************
*   \brief  Removes a job listener.
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
bool SJob::RemoveListener(SJobListener *pListener)
{
    if (pListener != NULL)
    {
        std::list<SJobListener *>::iterator iter = find(listeners.begin(), listeners.end(), pListener);
        if (iter != listeners.end())
        {
            listeners.erase(iter);
            return true;
        }
    }
    return false;
}

/**************************************************************************************
*   \brief  Destroys the job without release its memory
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
void SJob::Destroy()
{
    SetAlive(false);
    for (std::list<SJobListener *>::iterator iter = listeners.begin();
            iter != listeners.end();
            ++iter)
    {
        (*iter)->JobDestroyed(this);
    }
    listeners.clear();
}

/**************************************************************************************
*   \brief  Get stage specific data.
*
*   \version
*       - Sri Panyam  20/02/2009
*         Created
**************************************************************************************/
void *SJob::GetStageData(SStage *pStage)
{
    // if (pStage->ID() >= stageData.size()) return NULL;
    int id = pStage->ID();
    int size = stageData.size();
    for (int i = size;i <= id;i++)
        stageData.push_back(NULL);
    return stageData[id];
}

/**************************************************************************************
*   \brief  Set stage specific data.
*
*   \version
*       - Sri Panyam  20/02/2009
*         Created
**************************************************************************************/
void *SJob::SetStageData(SStage *pStage, void * data)
{
    int id = pStage->ID();
    int size = stageData.size();
    for (int i = size;i <= id;i++)
        stageData.push_back(NULL);
    void *old = stageData[id];
    stageData[id] = data;
    return old;
}

/**************************************************************************************
*   \brief  Sets the alive status.
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
void SJob::SetAlive(bool alive)
{
    isAlive = alive;
}

/**************************************************************************************
*   \brief  Gets the alive status.
*
*   \version
*       - Sri Panyam  20/04/2009
*         Created
**************************************************************************************/
bool SJob::IsAlive()
{
    return isAlive;
}


