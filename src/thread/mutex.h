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
 *  \file   mutex.h
 *
 *  \brief  Synchronization related classes.
 *
 *  \version
 *      - S Panyam      14/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SMUTEX_H_
#define _SMUTEX_H_

//*****************************************************************************
/*!
 *  \class  SMutex
 *
 *  \brief  A mutual exclusive object.
 *
 *****************************************************************************/
class SMutex
{
    friend class SMutexLock;
    friend class SCondition;

public:
    SMutex();
    SMutex(int kind);
    ~SMutex();
    int Lock(bool tryonly = false);
    void Unlock();
    template <typename T> T Get() { return (T)pMutex; }

private:
    void *pMutex;
    void *pMutexAttr;
};

//*****************************************************************************
/*!
 *  \class  SMutexLock
 *
 *  \brief  A wrapper for locking and unlocking mutual exclusive objects.
 *
 *****************************************************************************/
class SMutexLock
{
public:
    SMutexLock(SMutex &mutex);
    ~SMutexLock();

private:
    SMutexLock();
    SMutex *pTheMutex;
};
//
//*****************************************************************************
/*!
 *  \class  SCondition
 *
 *  \brief  A wrapper for condition variables.
 *
 *****************************************************************************/
class SCondition
{
public:
    SCondition(SMutex &mutex);
    ~SCondition();
    int Wait();
    int Signal();

private:
    pthread_cond_t condVar;
    SMutex *pTheMutex;
};

#endif

