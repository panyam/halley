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
 *  \file   mutex.cpp
 *
 *  \brief  Synchronization related classes.
 *
 *  \version
 *      - Sri Panyam      14/02/2009
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
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "thread/mutex.h"

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
SMutex::SMutex()
{
    pMutex      = malloc(sizeof(pthread_mutex_t));
    pMutexAttr  = NULL;
    pthread_mutex_init(reinterpret_cast<pthread_mutex_t *>(pMutex), NULL);
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  Sets the type of mutex as well.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
SMutex::SMutex(int kind)
{
    pMutex      = malloc(sizeof(pthread_mutex_t));
    pMutexAttr  = malloc(sizeof(pthread_mutexattr_t));
    pthread_mutexattr_settype(reinterpret_cast<pthread_mutexattr_t *>(pMutexAttr), kind);
    pthread_mutex_init(reinterpret_cast<pthread_mutex_t *>(pMutex), NULL);
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
SMutex::~SMutex()
{
    if (pMutex)
    {
        pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(pMutex));
        free(pMutex);
        if (pMutexAttr)
            free(pMutexAttr);
    }
}

//*****************************************************************************
/*!
 *  \brief  Aquires a lock on the mutex.  Blocks until the lock is
 *  provided.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SMutex::Lock(bool tryonly)
{
    pthread_mutex_t *pMut = reinterpret_cast<pthread_mutex_t *>(pMutex);

    if (tryonly)
    {
        return pthread_mutex_trylock(pMut);
    }
    else
    {
        return pthread_mutex_lock(pMut);
    }
}

//*****************************************************************************
/*!
 *  \brief  Releases the lock on the mutex.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SMutex::Unlock()
{
    pthread_mutex_t *pMut = reinterpret_cast<pthread_mutex_t *>(pMutex);
    pthread_mutex_unlock(pMut);
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
SMutexLock::SMutexLock(SMutex &mutex) : pTheMutex(&mutex)
{
    pTheMutex->Lock();
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
SMutexLock::~SMutexLock()
{
    pTheMutex->Unlock();
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
SCondition::SCondition(SMutex &mutex) : pTheMutex(&mutex)
{
    pthread_cond_init(&condVar, NULL);
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
SCondition::~SCondition()
{
    pthread_cond_destroy(&condVar);
}

//*****************************************************************************
/*!
 *  \brief  Waits on the condition variable.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SCondition::Wait(int milliseconds)
{
    if (milliseconds > 0)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        struct timespec stoptime;
        stoptime.tv_sec     = tv.tv_sec + milliseconds / 1000;
        stoptime.tv_nsec    = (tv.tv_usec * 1000) + ((milliseconds % 1000) * 1000000);
        if (stoptime.tv_nsec >= 1000000000)
        {
            stoptime.tv_nsec -= 1000000000;
            stoptime.tv_sec++;
        }

        int result = pthread_cond_timedwait(&condVar,
                                            reinterpret_cast<pthread_mutex_t *>(pTheMutex->pMutex),
                                            &stoptime);

        if (result == 0 || result == ETIMEDOUT)
            return 0;
        else
            return result;
    }
    else
    {
        return pthread_cond_wait(&condVar, reinterpret_cast<pthread_mutex_t *>(pTheMutex->pMutex));
    }
}

//*****************************************************************************
/*!
 *  \brief  Signals for the cond var to wake up.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SCondition::Signal()
{
    return pthread_cond_signal(&condVar);
}

