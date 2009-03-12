//*****************************************************************************
/*!
 *  \file   task.cpp
 *
 *  \brief  Common task class.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <signal.h>

#include "thread/task.h"

using std::cerr;
using std::endl;

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
STask::STask() :
    taskState(TASK_CREATED),
    taskRunningCond(taskStateMutex),
    taskDeadCond(taskStateMutex)
{
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
STask::~STask()
{
    Stop();
}

//*****************************************************************************
/*!
 *  \brief  Starts the Task and blocks until it is finished.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void STask::Start()
{
    {
        SMutexLock stateMutexLock(taskStateMutex);
        if (taskState == TASK_RUNNING)
        {
            cerr << "INFO: Task is already running." << endl << endl;
            return ;
        }
        else if (taskState == TASK_STARTED)
        {
            // wait for it to get to running state
            WaitForTaskBegin();
            return ;
        }
        else if (taskState == TASK_STOPPED)
        {
            // wait for it to fully stop before starting it
            WaitForTaskFinish();
        }
    }

    Run();
}

//*****************************************************************************
/*!
 *  \brief  Starts the Task asynchronously.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void STask::StartInThread()
{
    SMutexLock stateMutexLock(taskStateMutex);
    if (taskState == TASK_RUNNING)
    {
        cerr << "INFO: Task is already running." << endl << endl;
        return ;
    }
    else if (taskState == TASK_STARTED)
    {
        // wait for it to get to running state
        WaitForTaskBegin();
        return ;
    }
    else if (taskState == TASK_STOPPED)
    {
        // wait for it to fully stop before starting it
        WaitForTaskFinish();
    }

    // put task in started stated
    taskState = TASK_STARTED;

    {
        SMutexLock listenerLock(listenerMutex);
        for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
        {
            STaskListener *pListener = *iter;
            pListener->TaskStarted(this);
        }
    }

    pthread_create(&theTask, NULL, TaskStartFunc, this);

    // detach it so its resources can be reclaimed when it is done
    // instead of us having to join manually.
    // pthread_detach(theTask);

    // wait till task gets into the run state.
    WaitForTaskBegin();
}

//*****************************************************************************
/*!
 *  \brief  Tries to stop the task.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void STask::Stop()
{
    {
        SMutexLock stateMutexLock(taskStateMutex);

        if (taskState == TASK_STARTED)
        {
            WaitForTaskBegin();
        }
        else if (taskState == TASK_STOPPED)
        {
            WaitForTaskFinish();
        }

        if (taskState != TASK_RUNNING)
        {
            return ;
        }

        taskState = TASK_STOPPED;
    }

    {
        SMutexLock listenerLock(listenerMutex);
        for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
        {
            STaskListener *pListener = *iter;
            pListener->TaskStopped(this);
        }
    }

    std::cerr << "INFO: Stopping child: ..." << std::endl << std::endl;

    RealStop();

    // Join();

    // wait for the task to finish
    {
        SMutexLock stateMutexLock(taskStateMutex);
        WaitForTaskFinish();
    }
}

//*****************************************************************************
/*!
 *  \brief  Block till task finishes execution
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void STask::Join()
{
    if (IsRunning())
    {
        int status;
        pthread_join(theTask, (void **)&status);
    }
}

//*****************************************************************************
/*!
 *  \brief  Start function for a task.
 *
 *  \param  void * - The passed in to the task as its param.
 *
 *  \return void * - Return value. We just return NULL
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
void * STask::TaskStartFunc(void * pData)
{
    STask *pTheTask = static_cast<STask *>(pData);

    assert(pTheTask != NULL);

    {
        SMutexLock locker(pTheTask->taskStateMutex);
        if (pTheTask->taskState == TASK_STARTED)
            pTheTask->SignalTaskBegin();
        else
        {
            // what to do here?
            assert("Not sure what to do here" && false);
        }
    }

    int result = pTheTask->Run();

    {
        SMutexLock listenerLock(pTheTask->listenerMutex);
        for (Listeners::iterator iter = pTheTask->listeners.begin(); 
                iter != pTheTask->listeners.end();++iter)
        {
            STaskListener *pListener = *iter;
            pListener->TaskFinished(pTheTask, result);
        }
    }

    pTheTask->SignalTaskFinish();

    pthread_exit(NULL);

    return NULL;
}

//*****************************************************************************
/*!
 *  \brief  Tells if the task is running or not
 *
 *  \version
 *      - Sri Panyam      16/02/2009
 *        Created.
 *
 *****************************************************************************/
bool STask::IsRunning()
{
    SMutexLock stateMutexLock(taskStateMutex);
    return taskState >= TASK_STARTED && taskState <= TASK_STOPPED;
}

//*****************************************************************************
/*!
 *  \brief  Tells if the task has been stopped
 *
 *  \version
 *      - Sri Panyam      16/02/2009
 *        Created.
 *
 *****************************************************************************/
bool STask::Stopped()
{
    SMutexLock stateMutexLock(taskStateMutex);
    return taskState >= TASK_STOPPED;
}

//*****************************************************************************
/*!
 *  \brief  Returns true if this task is the current task!
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool STask::IsCurrent()
{
    return pthread_equal(theTask, pthread_self());
}

//*****************************************************************************
/*!
 *  \brief  If a task has been started but not yet in the running state,
 *  this blocks until the task has got into the running state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int STask::WaitForTaskBegin()
{
    cerr << "INFO: Task alreay started, waiting to fully start..." << endl << endl;
    return taskRunningCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  If a task has been stopped but still running, this blocks
 *  until the task has stopped running.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int STask::WaitForTaskFinish()
{
    cerr << "INFO: Waiting for Task to fully stop..." << endl << endl;
    return taskDeadCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  Signals that the task has gone to the start state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int STask::SignalTaskBegin()
{
    cerr << "INFO: Task gone to start state!" << endl << endl;

    taskState = TASK_RUNNING;

    return taskRunningCond.Signal();
}

//*****************************************************************************
/*!
 *  \brief  Signals that the task has gone to the finished state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int STask::SignalTaskFinish()
{
    cerr << "INFO: Task gone to finish state!" << endl << endl;

    SMutexLock stateMutexLock(taskStateMutex);

    taskState = TASK_TERMINATED;

    return taskDeadCond.Signal();
}

