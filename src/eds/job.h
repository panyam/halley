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
 *  \file   job.h
 *
 *  \brief
 *  A job that is handled in different stages.
 *
 *  \version
 *      - Sri Panyam      20/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEDS_JOB_H_
#define _SEDS_JOB_H_

#include "eds/fwd.h"

//*****************************************************************************
/*!
 *  \class  SJob
 *
 *  \brief A job that is handled in different stages.
 *
 *****************************************************************************/
class SJob
{
public:
    //! Destroys the connection object
    SJob();

    //! Destroys the connection object
    virtual ~SJob();

    //! Destroys the job.
    virtual void Destroy();

    //! Get the stage specific data for this connection
    void *  GetStageData(SStage *pStage);

    //! Set the stage specific data for this connection
    void *  SetStageData(SStage *pStage, void * data);

    //! Tells if the job is alive.
    bool IsAlive();

    //! Sets the alive status of the job
    void SetAlive(bool alive);

private:
    //! Connection specific data that handlers can manipulate to their will.
    std::vector<void *>     stageData;

    //! Is the connection alive?
    bool                isAlive;
};

#endif

