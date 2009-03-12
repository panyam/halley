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

//*****************************************************************************
/*!
 *  \class  STask
 *
 *  \brief  Common task class.
 *
 *****************************************************************************/
class STask
{
public:
    // Constructor
    STask() : taskStopped(false), taskRunning(false) { }

    // Destructor
    virtual ~STask();

    // Starts the task.
    int     Start();

    // Tries to stop the task
    void    Stop();

    // Tells if the task has been stopped.
    bool    Running();

    // Tells if the task has been stopped.
    bool    Stopped();

protected:
    //! Main task run function
    virtual int Run()           =   0;

    //! Main task stop function
    virtual int  RealStop()     =   0;

private:
    //! Tells whether the task has been stopped or not
    bool    taskStopped;

    //! Tells if the task is running
    bool    taskRunning;
};

#endif

