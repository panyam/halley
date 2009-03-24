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

//! A module that sends data as it comes from several data sources
class SBayeuxChannel
{
public:
    //! Destructor
    virtual ~SBayeuxChannel() { }

    //! Get the channel name
    const SString &Name() const { return name; }

protected:
    SString name;
};

#endif

