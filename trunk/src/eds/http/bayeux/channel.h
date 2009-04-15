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
#include "../../../json/json.h"

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
    //! Constructor
    SBayeuxChannel(const std::string &n, SBayeuxModule *pMod = NULL)
        : pModule(pMod), name(n) { }

    //! Destructor
    virtual ~SBayeuxChannel() { }

    //! Sets the module we belong to
    virtual void SetBayeuxModule(SBayeuxModule *pMod) { pModule = pMod; }

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

    //! The module to which this channel belongs
    SBayeuxModule *pModule;

    //! Name of the channel
    SString name;
};

#endif

