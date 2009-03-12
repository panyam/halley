//*****************************************************************************
/*!
 *  \file   task.h
 *
 *  \brief  Common task class - can run synchronously or asynchronously.
 *
 *  \version
 *      - S Panyam      16/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _STASK_H_
#define _STASK_H_

#include <list>
#include "mutex.h"
#include "utils/listeners.h"
#include "listeners.h"

//*****************************************************************************
/*!
 *  \class  STask
 *
 *  \brief  Common task class.
 *
 *****************************************************************************/
class STask : public SLockableListenerManager<STaskListener>
{
public:
    typedef enum
    {
        TASK_CREATED,
        TASK_STARTED,
        TASK_RUNNING,
        TASK_STOPPED,
        TASK_FINISHED,
    } TaskState;

public:
    // Constructor
    STask();

    // Destructor
    virtual ~STask();

    // Starts the task.
    void    Start();

    // Starts the task asynchronously
    void    StartInThread();

    // Tries to stop the task
    void    Stop();

    // Tells if the task is still running
    bool    IsRunning();

    // Tells if the task has been stopped.
    bool    Stopped();

    //! Wait till task finishes execution
    void    Join();
    
    //! Returns the task ID
    bool    IsCurrent();

protected:
    //! Start function for the http server.
    static void * TaskStartFunc(void * pData);

    //! Main task run function
    virtual int Run()           =   0;

    //! Main task stop function
    virtual int  RealStop()     =   0;

private:
    // Waits for a stopped task to finish running.
    int             WaitForTaskBegin();

    // Waits for a started task to get into "running" state
    int             WaitForTaskFinish();

    // Signals that the task has gone to the "running" state
    int             SignalTaskBegin();

    // Signals that the task has gone to the "dead" state
    int             SignalTaskFinish();

private:
    //! The actual task item
    pthread_t       theTask;

    //! State of the task
    TaskState     taskState;

    //! Mutex variable for the task state
    SMutex          taskStateMutex;

    //! Condition variable for waiting for task to go to the "running" state
    SCondition      taskRunningCond;

    //! Condition variable for waiting for task to go to the "dead" state
    SCondition      taskDeadCond;
};

#endif

