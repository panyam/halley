//*****************************************************************************
/*!
 *  \file   channel.h
 *
 *  \brief  A bayeux channel.
 *
 *  \version
 *      - S Panyam      24/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SBAYEUX_CHANNEL_H_
#define _SBAYEUX_CHANNEL_H_

#include "../httpfwd.h"

//! Handles events sent to a channel
class SChannelListener
{
public:
    // Destructor
    virtual ~SChannelListener() { }

    //! Handles an event.
    virtual void HandleEvent(const JsonNodePtr &event, JsonNodePtr &output) { }
};

//! A module that sends data as it comes from several data sources
class SBayeuxChannel
{
public:
    //! Destructor
    virtual ~SBayeuxChannel() { }

    //! Sets the channel listener
    void SetChannelListener(SChannelListener *pList) { pListener = pList; }

    //! Handles an event
    void HandleEvent(const JsonNodePtr &event, JsonNodePtr &output)
    {
        if (pListener)
            pListener->HandleEvent(event, output);
    }

    //! Get the channel name
    const SString &Name() const { return name; }

protected:
    //! Constructor with name
    SBayeuxChannel(const SString &n) : name(n) { }

protected:
    //! The listener
    SChannelListener *  pListener;

    //! Name of the channel
    SString name;
};

#endif

