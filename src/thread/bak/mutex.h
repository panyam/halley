//*****************************************************************************
/*!
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
    void Lock();
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

