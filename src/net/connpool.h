

#ifndef SCONN_POOL_H
#define SCONN_POOL_H

#include "connfactory.h"
#include "connhandler.h"
#include <list>
#include <algorithm>

//*****************************************************************************
/*!
 *  \class  SConnPool
 *
 *  \brief  A factory that holds handlers in a pool.
 *
 *****************************************************************************/
template <typename T>
class SConnPool : public SConnFactory
{
protected:
    //! List of clients
    std::list<T *>              clients;

    //! A mutex for the client list
    SMutex                      clientListMutex;

public:
    //*****************************************************************************
    /*!
     *  \brief  Return a new instance of a client handle.
     *
     *  \version
     *      - Sri Panyam      10/02/2009
     *        Created.
     *
     *****************************************************************************/
    virtual T *  NewHandler()
    {
        SMutexLock    mutexLock(clientListMutex);

        T *pHandler = NULL;
        for (typename std::list<T *>::iterator iter=clients.begin();
                iter != clients.end() && pHandler != NULL;
                ++iter)
        {
            pHandler = dynamic_cast<T*>(*iter);
            if (pHandler == NULL || (*iter)->Running())
            {
                pHandler = NULL;
            }
        }

        if ( ! pHandler)
        {
            pHandler = new T();
            clients.push_back(pHandler);
        }
        return pHandler;
    }

    //*****************************************************************************
    /*!
     *  \brief  Releases an instance of a client handle.
     *
     *  \version
     *      - Sri Panyam      10/02/2009
     *        Created.
     *
     *****************************************************************************/
    virtual void            ReleaseHandler(SConnHandler * handler)
    { 
        // TODO: Assert if handler is not of type T.
        SMutexLock    mutexLock(clientListMutex);
        T * t = dynamic_cast<T *>(handler);
        if (t != NULL)
            if (find(clients.begin(), clients.end(), t) == clients.end())
                clients.push_back(t);
    }
};

#endif

