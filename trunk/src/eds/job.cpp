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
    // create space for the first 5 stages
    for (int i = 0;i < 5;i++)
    {
        stageData.push_back(NULL);
    }
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
void SJob::SetStageData(SStage *pStage, void * data)
{
    int id = pStage->ID();
    int size = stageData.size();
    for (int i = size;i <= id;i++)
        stageData.push_back(NULL);
    stageData[id] = data;
}


