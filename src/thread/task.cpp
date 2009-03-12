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
 *  \brief  Tries to stop the task.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int STask::Start()
{
    taskRunning = true;
    taskStopped = false;
    int result = Run();
    taskRunning = false;
    return result;
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
    taskStopped = true;

    RealStop();
}

//*****************************************************************************
/*!
 *  \brief  Tells if the task is running.
 *
 *  \version
 *      - Sri Panyam      18/02/2009
 *        Created.
 *
 *****************************************************************************/
bool STask::Running()
{
    return taskRunning;
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
    return taskStopped;
}

