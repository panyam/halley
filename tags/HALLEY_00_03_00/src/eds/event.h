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
 *  \file   event.h
 *
 *  \brief  Events!
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_H_
#define _SEVENT_H_

#include "eds/fwd.h"

//*****************************************************************************
/*!
 *  \class  Holds all info for an event.
 *
 *  \brief  An event queue used by event handlers and dispatchers.
 *
 *****************************************************************************/
class SEvent
{
public:
    //! Creates a new event with defaults
    SEvent(int type = -1, SJob *pSource = NULL, void *data = NULL, int priority_ = 1000)
    {
        Reset(type, pSource, data, priority_);
    }

    //! Reset the event.
    void Reset(int type = -1, SJob *pSource_ = NULL, void *data = NULL, int priority_ = 1000)
    {
        // default priority = 1000
        priority    = priority_;
        evType      = type;
        pSource     = pSource_;
        pData       = data;
    }

    //! Return the extra data.
    template <typename T> T Data() const { return reinterpret_cast<T>(pData); }

    //! event type - app specific
    int     evType;

    //! The source that is responsible for this event.
    SJob *  pSource;

    //! The actual event data - can be a bucket, string, anything really -
    // specific to the module and event type
    void *  pData;

    //! Priority of the event
    int     priority;
};

// overloaded to do comparison between 2 events
extern bool operator<(const SEvent &a, const SEvent &b);

#endif

