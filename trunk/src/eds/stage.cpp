//*****************************************************************************
/*!
 *  \file   stage.cpp
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

#include "stage.h"
#include "handler.h"

//! The dispatcher handles events as they arrive.
class SEventDispatcher : public STask
{
public:
    //! Creates and resets the event dispatcher
    SEventDispatcher(SStage *pStager);

protected:
    //! Handles events continuosly
    int Run();

    //! Does things when needed to stop the task
    int RealStop() { return 0; }

private:
    //! The stage in which this handler belongs to
    SStage *pStage;
};

//! Number of stages in the system
int SStage::STAGE_COUNTER = 0;

//! Creates and resets the event dispatcher
SEventDispatcher::SEventDispatcher(SStage *stage)
:
    pStage(stage)
{
}

//! Handles events continuosly
int SEventDispatcher::Run()
{
    while (!Stopped())
    {
        // get the event
        SEvent event = pStage->GetEvent();

        // pHandler->HandleEvent(event);
        pStage->HandleEvent(event);
    }
    return 0;
}

//! Creates a new stage
SStage::SStage(int numThreads)
:
    evtQueueCondition(evtQueueMutex)
{
    // increment stage count!
    stageID = STAGE_COUNTER++;

    for (int i = 0;i < numThreads;i++)
    {
        handlerThreads.push_back(NULL);
    }
}

//! Destroys the stage - Stop MUST be called before destruction
SStage::~SStage()
{
    for (int i = 0, numThreads = handlerThreads.size();i < numThreads;i++)
    {
        if (handlerThreads[i] != NULL)
        {
            assert("Stage::Stop MUST be called before destruction." && !handlerThreads[i]->Running());
        }
    }
}

//! Starts the stage
void SStage::Start()
{
    if (!handlerThreads.empty())
    {
        for (int i = 0, numThreads = handlerThreads.size();i < numThreads;i++)
        {
            if (handlerThreads[i] != NULL)
            {
                handlerThreads[i] = new SThread(new SEventDispatcher(this));
            }
            handlerThreads[i]->Start();
        }
    }
}

//! Stop the stage and all its threads
void SStage::Stop()
{
    if (!handlerThreads.empty())
    {
        for (int i = 0, numThreads = handlerThreads.size();i < numThreads;i++)
        {
            if (handlerThreads[i] != NULL)
            {
                handlerThreads[i]->Stop();
            }
        }
    }
}

//! Queue an event to be handled later
void SStage::QueueEvent(const SEvent &event)
{
    if (handlerThreads.empty())
    {
        this->HandleEvent(event);
    }
    else
    {
        SMutexLock locker(evtQueueMutex);

        eventQueue.push(event);

        // signal waiting threads to wakeup
        evtQueueCondition.Signal();
    }
}

//! Queue an event to be handled later
SEvent SStage::GetEvent()
{
    assert("Handler thread is empty" && !handlerThreads.empty());

    SMutexLock locker(evtQueueMutex);

    // wait while queue is empty
    while (eventQueue.empty())
        evtQueueCondition.Wait();

    SEvent out = eventQueue.top();

    eventQueue.pop();

    return out;
}

