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
    STask() : taskStopped(false), taskRunning(false), taskDead(false) { }

    // Destructor
    virtual ~STask();

    // Starts the task.
    int     Start();

    // Tries to stop the task
    void    Stop();

    // Tells if the task has been stopped.
    bool    Running();

    // Tells if the task is died or not
    bool    Dead();

    // Tells if the task has been stopped.
    bool    Stopped();

protected:
    //! Main task run function
    virtual int Run() { return 0; }

    //! Main task stop function
    virtual int  RealStop() { return 0; }

private:
    //! Tells whether the task has been stopped or not
    bool    taskStopped;

    //! Tells if the task is running
    bool    taskRunning;

    //! Tells if task died or finished normally
    bool    taskDead;
};

#endif

