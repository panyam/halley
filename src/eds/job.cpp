//*****************************************************************************
/*!
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
*   \brief  Get stage specific data.
*
*   \version
*       - Sri Panyam  20/02/2009
*         Created
**************************************************************************************/
void *SJob::GetStageData(SStage *pStage)
{
    int id = pStage->ID();
    for (int i = stageData.size();i <= id;i++)
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
    for (int i = stageData.size();i <= id;i++)
        stageData.push_back(NULL);
    stageData[id] = data;
}


