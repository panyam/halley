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
 *  \file   listeners.h
 *
 *  \brief  Abstract listener classes.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef SLISTENER_MANAGER_H
#define SLISTENER_MANAGER_H

#include <algorithm>
#include <list>
#include "thread/mutex.h"

template <typename ListenerClass>
class SListenerManager
{
public:
    //! Virtual destructor to avoid warnings with some compilers.
    virtual ~SListenerManager() { listeners.clear(); }

    //! Add a listener for this handler
    virtual void            AddListener(ListenerClass *pListener);

    //! Remove
    virtual void            RemoveListener(ListenerClass *pListener);

    //! Returns number of listeners in this table.
    virtual int             Count();

protected:
    //! Shortcut to the listener list type
    typedef std::list<ListenerClass *>  Listeners;

    //! List of listeners for this handler
    std::list<ListenerClass *>   listeners;
};

template <typename ListenerClass>
class SLockableListenerManager : public SListenerManager<ListenerClass>
{
public:
    //! Add a listener for this handler
    virtual void            AddListener(ListenerClass *pListener);

    //! Remove
    virtual void            RemoveListener(ListenerClass *pListener);

    //! Returns number of listeners in this table.
    virtual int             Count();

protected:
    //! Mutex for the listener list
    SMutex  listenerMutex;
};

/**************************************************************************************
*   \brief  Adds a listener.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
void SListenerManager<ListenerClass>::AddListener(ListenerClass *pListener)
{
    if (find(listeners.begin(), listeners.end(), pListener) != listeners.end())
        return ;
    listeners.push_back(pListener);
}

/**************************************************************************************
*   \brief  Removes a listener.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
void SListenerManager<ListenerClass>::RemoveListener(ListenerClass *pListener)
{
    if (find(listeners.begin(), listeners.end(), pListener) == listeners.end())
        return ;
    listeners.remove(pListener);
}

/**************************************************************************************
*   \brief  Gets the number of listeners present.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
int SListenerManager<ListenerClass>::Count()
{
    return listeners.size();
}

/**************************************************************************************
*   \brief  Locks the listener table while adding a listener.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
void SLockableListenerManager<ListenerClass>::AddListener(ListenerClass *pListener)
{
    SMutexLock locker(listenerMutex);
    SListenerManager<ListenerClass>::AddListener(pListener);
}

/**************************************************************************************
*   \brief  Locks the listener table while removing a listener.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
void SLockableListenerManager<ListenerClass>::RemoveListener(ListenerClass *pListener)
{
    SMutexLock locker(listenerMutex);
    SListenerManager<ListenerClass>::RemoveListener(pListener);
}

/**************************************************************************************
*   \brief  Locks the listener table while getting the size of the listeners.
*
*   \version
*       - Sri Panyam  13/02/2009
*         Created
**************************************************************************************/
template <typename ListenerClass>
int SLockableListenerManager<ListenerClass>::Count()
{
    SMutexLock locker(listenerMutex);
    return SListenerManager<ListenerClass>::Count();
}

#endif

