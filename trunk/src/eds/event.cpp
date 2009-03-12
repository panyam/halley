//*****************************************************************************
/*!
 *  \file   evqueue.h
 *
 *  \brief  A queue of events.
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#include "event.h"

// overloaded to do comparison between 2 events
bool operator<(const SEvent &a, const SEvent &b)
{
    return a.priority < b.priority;
}

