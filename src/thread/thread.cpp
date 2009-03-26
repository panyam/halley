//*****************************************************************************
/*!
 *  \file   thread.cpp
 *
 *  \brief  Common thread class.
 *
 *  \version
 *      - S Panyam      18/02/2009
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
#include "thread/thread.h"

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
SThread::SThread(STask *task) :
    pTask(task),
    threadState(THREAD_CREATED),
    threadRunningCond(threadStateMutex),
    threadDeadCond(threadStateMutex)
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
SThread::~SThread()
{
    Stop();
}

//*****************************************************************************
/*!
 *  \brief  Starts the Thread
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SThread::Start()
{
    SMutexLock stateMutexLock(threadStateMutex);
    if (threadState == THREAD_RUNNING)
    {
        return ;
    }
    else if (threadState == THREAD_STARTED)
    {
        // wait for it to get to running state
        WaitForThreadBegin();
        return ;
    }
    else if (threadState == THREAD_STOPPED)
    {
        // wait for it to fully stop before starting it
        WaitForThreadFinish();
    }

    // put task in started stated
    threadState = THREAD_STARTED;

    {
        SMutexLock listenerLock(listenerMutex);
        for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
        {
            (*iter)->ThreadStarted(this);
        }
    }

    pthread_create(&theThread, NULL, ThreadStartFunc, this);

    // detach it so its resources can be reclaimed when it is done
    // instead of us having to join manually.
    // pthread_detach(theTask);

    // wait till task gets into the run state.
    WaitForThreadBegin();
}

//*****************************************************************************
/*!
 *  \brief  Tries to stop the thread.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SThread::Stop()
{
    SMutexLock stateMutexLock(threadStateMutex);

    if (threadState == THREAD_STARTED)
    {
        WaitForThreadBegin();
    }
    else if (threadState == THREAD_STOPPED)
    {
        WaitForThreadFinish();
    }

    if (threadState != THREAD_RUNNING)
    {
        return ;
    }

    threadState = THREAD_STOPPED;

    if (pTask != NULL)
    {
        pTask->Stop();

        {
            SMutexLock listenerLock(listenerMutex);
            for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
            {
                (*iter)->ThreadStopped(this);
            }
        }

        WaitForThreadFinish();
    }
}

//*****************************************************************************
/*!
 *  \brief  Block till thread finishes execution
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SThread::Join()
{
    if (IsRunning())
    {
        pthread_join(theThread, NULL);
    }
}

//*****************************************************************************
/*!
 *  \brief  Main thread function.
 *
 *  \param  void * - The passed in to the thread as its param.
 *
 *  \return void * - Return value. We just return NULL
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SThread::Run()
{
    SignalThreadBegin();

    {
        SMutexLock listenerLock(listenerMutex);
        for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
        {
            SThreadListener *pListener = *iter;
            pListener->ThreadStarted(this);
        }
    }

    if (pTask)
    {
        int result = pTask->Start();

        {
            SMutexLock listenerLock(listenerMutex);
            for (Listeners::iterator iter = listeners.begin(); iter != listeners.end();++iter)
            {
                SThreadListener *pListener = *iter;
                pListener->ThreadFinished(this, result);
            }
        }
    }

    SignalThreadFinish();
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
void * SThread::ThreadStartFunc(void * pData)
{
    SThread *pTheThread = static_cast<SThread *>(pData);

    assert(pTheThread != NULL);

    pTheThread->Run();

    pthread_exit(NULL);

    return NULL;
}

//*****************************************************************************
/*!
 *  \brief  Tells if the thread is running or not
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool SThread::IsRunning()
{
    SMutexLock stateMutexLock(threadStateMutex);
    return threadState >= THREAD_STARTED && threadState <= THREAD_STOPPED;
}

//*****************************************************************************
/*!
 *  \brief  Returns true if this thread is the current thread!
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool SThread::IsCurrent()
{
    return pthread_equal(theThread, pthread_self());
}

//*****************************************************************************
/*!
 *  \brief  If a thread has been started but not yet in the running state,
 *  this blocks until the thread has got into the running state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SThread::WaitForThreadBegin()
{
    return threadRunningCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  If a thread has been stopped but still running, this blocks
 *  until the thread has stopped running.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SThread::WaitForThreadFinish()
{
    return threadDeadCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  Signals that the thread has gone to the start state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SThread::SignalThreadBegin()
{
    SMutexLock locker(threadStateMutex);
    if (threadState == THREAD_STARTED)
    {
        threadState = THREAD_RUNNING;
        return threadRunningCond.Signal();
    }
    else
    {
        // what to do here?
        assert("Not sure what to do here" && false);
    }
    return -1;
}

//*****************************************************************************
/*!
 *  \brief  Signals that the thread has gone to the finished state.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SThread::SignalThreadFinish()
{
    SMutexLock stateMutexLock(threadStateMutex);

    threadState = THREAD_FINISHED;

    return threadDeadCond.Signal();
}

