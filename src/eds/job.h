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
#include "utils/refcount.h"

//*****************************************************************************
/*!
 *  \class  SJobListener
 *
 *  \brief  Handles events from a job.
 *
 *****************************************************************************/
class SJobListener
{
public:
    //! Destructor
    virtual ~SJobListener() { }

    //! Called when the job id destroyed
    virtual void JobDestroyed(SJob *pJob) { }
};

//*****************************************************************************
/*!
 *  \class  SJob
 *
 *  \brief A job that is handled in different stages.
 *
 *****************************************************************************/
class SJob : public RefCountable
{
public:
    //! Creates a new job
    SJob();

    //! Destroys the connection object
    virtual ~SJob();

    //! Get the stage specific data for this connection
    void *GetStageData(SStage *pStage);

    //! Set the stage specific data for this connection
    void *SetStageData(SStage *pStage, void * data);

    //! Adds a job listener
    virtual bool AddListener(SJobListener *pListener);

    //! Removes a job listener
    virtual bool RemoveListener(SJobListener *pListener);

    /*
    //! Increase reference count
    virtual void IncRef(unsigned delta = 1)
    {
        RefCountable::IncRef(delta);
        SLogger::Get()->Log("DEBUG: IncRefed %x to %d\n", this, Count());
    }

    //! Decrease reference count
    // Returns true if reference count reaches 0
    virtual bool    DecRef(unsigned delta = 1)
    {
        bool result = RefCountable::DecRef(delta);
        SLogger::Get()->Log("DEBUG: DecRefed %x to %d\n", this, Count());
        return result;
    }
    */

private:
    //! Connection specific data that handlers can manipulate to their will.
    std::vector<void *>         stageData;

    //! Job listeners
    std::list<SJobListener *>   listeners;
};

#endif

