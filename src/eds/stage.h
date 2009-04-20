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
 *  \file   stage.h
 *
 *  \brief  A stage that stores events in a queue and palms them off
 *  to handlers as necessary.  Also maintains a pool of handlers so they
 *  can handle the event when they become free.
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_STAGE_H_
#define _SEVENT_STAGE_H_

#include <queue>
#include "thread/thread.h"
#include "eds/event.h"

//*****************************************************************************
/*!
 *  \class  SStageData
 *
 *  \brief  Data that belongs to a stage.
 *****************************************************************************/
class SStageData
{
public:
    virtual ~SStageData() { }
protected:
    SConnection *pConnection;
};

//*****************************************************************************
/*!
 *  \class  SStage
 *
 *  \brief  A dispatcher that stores events in a queue and palms them off
 *  to handlers as necessary.  Also maintains a pool of handlers so they
 *  can handle the event when they become free.
 *
 *****************************************************************************/
class SStage
{
public:
    // Creates a new handler
    SStage(int numThreads = 0);
    
    // Destroys the stage - MUST call Stop before this
    virtual ~SStage();

    // Start the stage
    virtual void Start();

    // Stop the stage
    virtual void Stop();

    // Put an event in the queue, to be handled.
    virtual void QueueEvent(const SEvent &event);

    //! Pops and gets the next event in the queue
    virtual SEvent GetEvent();

    //! Get the stage ID.
    int ID() { return stageID; }

protected:
    friend class SEventDispatcher;

    //! Called to actually process the event
    virtual void HandleEvent(const SEvent &event) { }

private:
    //! Mutex on the event queue
    SMutex                  evtQueueMutex;

    //! A condition variable that waits when the queue is empty
    SCondition              evtQueueCondition;

    //! Event queues for unhandled events.
    std::priority_queue<SEvent>    eventQueue;

    //! The threads that will handle the events
    std::vector<SThread *>  handlerThreads;

private:
    //! Stage ID
    int     stageID;

    //! Number of stages in the system
    static int STAGE_COUNTER;
};

#endif

