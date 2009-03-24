//*****************************************************************************
/*!
 *  \file   bayeuxmodule.h
 *
 *  \brief  A module for handling bayeux connections.  Hello Comet!!!
 *
 *  \version
 *      - S Panyam      23/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SBAYEUX_MODULE_H_
#define _SBAYEUX_MODULE_H_

#include "../httpmodule.h"
#include "json/json.h"

class SBayeuxChannel;

//! A module that sends data as it comes from several data sources
class SBayeuxModule : public SHttpModule
{
public:
    // Constructor
    SBayeuxModule(SHttpModule *pNext) : SHttpModule(pNext) { }

    //! Registers a channel
    virtual bool RegisterChannel(SBayeuxChannel *pChannel, bool replace = false);

    //! Removes a channel
    virtual bool UnregisterChannel(const SBayeuxChannel *pChannel);

    //! Removes a channel by name
    virtual bool UnregisterChannel(const std::string &name);

    //! Called to handle connections
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage, 
                              SBodyPart *           pBodyPart);

protected:
    bool ProcessMessage(const JsonNodePtr &node, JsonNodePtr &output);
    bool ProcessHandshake(const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessConnect(const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessDisconnect(const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessSubscribe(const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessUnsubscribe(const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessMetaMessage(const std::string &channel, const JsonNodePtr &message, JsonNodePtr &output);
    bool ProcessPublish(const std::string &channel, const JsonNodePtr &message, JsonNodePtr &output);


protected:
    typedef std::map<std::string, SBayeuxChannel *> ChannelMap;

    //! Channels that are currently registered
    ChannelMap      channels;

    //! A list of data source subscriptions
    std::map<std::string, std::list<SConnection *> >    subscriptions;
};

#endif

