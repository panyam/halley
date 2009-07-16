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
    taskDead    = false;
    int result = 0;
    try
    {
        result = Run();
    }
    catch (...)
    {
        result      = -1;
        taskDead    = true;
    }
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

//*****************************************************************************
/*!
 *  \brief  Tells if the task finished normally or died.
 *
 *  \version
 *      - Sri Panyam      04/05/2009
 *        Created.
 *
 *****************************************************************************/
bool STask::Dead()
{
    return taskDead;
}

