//*****************************************************************************
/*!
 *  \file   event.h
 *
 *  \brief  Events!
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_QUEUE_H_
#define _SEVENT_QUEUE_H_

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
    SEvent(int type = -1, SJob *pJob = NULL, void *data = NULL, int priority_ = 1000)
    {
        Reset(type, pJob, data, priority_);
    }

    //! Reset the event.
    void Reset(int type = -1, SJob *pJob_ = NULL, void *data = NULL, int priority_ = 1000)
    {
        // default priority = 1000
        priority    = priority_;
        evType      = type;
        pJob        = pJob_;
        pData       = data;
    }

    //! Return the extra data.
    template <typename T> T Data() const { return reinterpret_cast<T>(pData); }

    //! event type - app specific
    int     evType;

    //! The job that is responsible for this event.
    SJob *  pJob;

    //! The actual event data - can be a bucket, string, anything really -
    // specific to the module and event type
    void *  pData;

    //! Priority of the event
    int     priority;
};

// overloaded to do comparison between 2 events
extern bool operator<(const SEvent &a, const SEvent &b);

#endif

