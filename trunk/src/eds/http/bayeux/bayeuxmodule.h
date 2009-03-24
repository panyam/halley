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
    SBayeuxModule(SHttpModule *pNext, const SString &b) : SHttpModule(pNext), boundary(b) { }

    //! Registers a channel
    virtual bool RegisterChannel(SBayeuxChannel *pChannel, bool replace = false);

    //! Removes a channel
    virtual bool UnregisterChannel(const SBayeuxChannel *pChannel);

    //! Removes a channel by name
    virtual bool UnregisterChannel(const SString &name);

    //! Called to handle connections
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage, 
                              SBodyPart *           pBodyPart);

protected:
    int  ProcessMessage(const JsonNodePtr & node,
                        JsonNodePtr &       output,
                        SConnection *       pConnection);

    int  ProcessHandshake(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessConnect(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessDisconnect(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessSubscribe(const JsonNodePtr &   message,
                          JsonNodePtr &         output,
                          SConnection *         pConnection);

    int  ProcessUnsubscribe(const JsonNodePtr & message,
                            JsonNodePtr &       output,
                            SConnection *       pConnection);

    int  ProcessMetaMessage(const SString & channel,
                            const JsonNodePtr & message,
                            JsonNodePtr &       output,
                            SConnection *       pConnection);

    int  ProcessPublish(const SString &         channel,
                        const JsonNodePtr &     message,
                        JsonNodePtr &           output,
                        SConnection *           pConnection);

    bool AddSubscription(const SString &channel, SConnection *pConnection);
    bool RemoveSubscription(const SString &channel, SConnection *pConnection);

protected:
    //! Collection of channels
    typedef std::map<SString, SBayeuxChannel *>     ChannelMap;

    //! Collection of subscriptions for each channel
    typedef std::map<SString, SConnectionList *>    ChannelSubscription;

    //! Channels that are currently registered
    ChannelMap              channels;

    //! A list of channel subscriptions
    ChannelSubscription     subscriptions;

    //! The boundary to be used bw multi part messages
    SString                 boundary;
};

#endif

