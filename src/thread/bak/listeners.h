//*****************************************************************************
/*!
 *  \file   listeners.h
 *
 *  \brief  Listeners in the thread library.
 *
 *  \version
 *      - S Panyam      15/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef STHREAD_LISTENERS_H
#define STHREAD_LISTENERS_H

class STask;
class SThread;

//*****************************************************************************
/*!
 *  \class  STaskListener
 *
 *  \brief  Listener of task events - mainly state changes.
 *
 *****************************************************************************/
class STaskListener
{
public:
    //! Default virtual destructor
    virtual ~STaskListener() { }

    //! Called just before a task starts
    virtual void    TaskStarted(STask *pTask)               = 0;

    //! Called if task has been stopped
    virtual void    TaskStopped(STask *pTask)               = 0;

    //! Called just after task finishes executing
    virtual void    TaskFinished(STask *pTask, int result)  = 0;
};

#endif

