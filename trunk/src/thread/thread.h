//*****************************************************************************
/*!
 *  \file   thread.h
 *
 *  \brief  Common thread class.
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _STHREAD_H_
#define _STHREAD_H_

#include <list>
#include "utils/listeners.h"
#include "task.h"
#include "mutex.h"

class SThread;

//*****************************************************************************
/*!
 *  \class  SThreadListener
 *
 *  \brief  Listener of thread events - mainly state changes.
 *
 *****************************************************************************/
class SThreadListener
{
public:
    //! Default virtual destructor
    virtual ~SThreadListener() { }

    //! Called just before a thread starts
    virtual void    ThreadStarted(SThread *pThread)               = 0;

    //! Called if thread has been stopped
    virtual void    ThreadStopped(SThread *pThread)               = 0;

    //! Called just after thread finishes executing
    virtual void    ThreadFinished(SThread *pThread, int result)  = 0;
};

//*****************************************************************************
/*!
 *  \class  SThread
 *
 *  \brief  Common thread class.
 *
 *****************************************************************************/
class SThread : public SLockableListenerManager<SThreadListener>
{
public:
    typedef enum
    {
        THREAD_CREATED,
        THREAD_STARTED,
        THREAD_RUNNING,
        THREAD_STOPPED,
        THREAD_FINISHED,
    } ThreadState;

public:
    // Constructor
    SThread(STask *task = NULL);

    // Destructor
    virtual ~SThread();

    //! Get the thread we are running.
    STask * GetTask() { return pTask; }

    // Starts the thread.
    void    Start();

    // Tries to stop the thread
    void    Stop();

    // Tells if the thread is running or not
    bool    IsRunning();

    //! Wait till thread finishes execution
    void    Join();
    
    //! Returns the thread ID
    bool    IsCurrent();

protected:
    //! Start function for the http server.
    static void *   ThreadStartFunc(void * pData);

    //! Does the thread stuff.
    void   Run();

private:
    // Waits for a stopped Thread to finish running.
    int             WaitForThreadBegin();

    // Waits for a started Thread to get into "running" state
    int             WaitForThreadFinish();

    // Signals that the Thread has gone to the "running" state
    int             SignalThreadBegin();

    // Signals that the Thread has gone to the "dead" state
    int             SignalThreadFinish();

private:
    //! The thread we are running
    STask *         pTask;

    //! State of the current thread.
    ThreadState     threadState;

    //! Mutex variable for the thread state
    SMutex          threadStateMutex;

    //! Condition variable for waiting for thread to go to the "running" state
    SCondition      threadRunningCond;

    //! Condition variable for waiting for thread to go to the "dead" state
    SCondition      threadDeadCond;

    //! The actual thread item
    pthread_t       theThread;
};

#endif
