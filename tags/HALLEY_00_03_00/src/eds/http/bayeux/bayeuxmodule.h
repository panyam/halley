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
    SBayeuxModule(SHttpModule *pNext, const SString &b)
        : SHttpModule(pNext), pHandlerStage(NULL), boundary(b) { }

    //! Registers a channel
    virtual bool RegisterChannel(SBayeuxChannel *pChannel, bool replace = false);

    //! Get a channel by name
    virtual SBayeuxChannel *GetChannel(const SString &name);

    //! Removes a channel
    virtual bool UnregisterChannel(const SBayeuxChannel *pChannel);

    //! Removes a channel by name
    virtual bool UnregisterChannel(const SString &name);

    //! Called to handle connections
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage, 
                              SBodyPart *           pBodyPart);

    //! Deliver an event to all connections
    virtual void DeliverEvent(const SBayeuxChannel *pChannel, const JsonNodePtr &value);

protected:
    //! Sends the response for a message
    void SendResponse(int                   result,
                      const JsonNodePtr &   output,
                      SHttpHandlerStage *   pStage, 
                      SHttpHandlerData *    pHandlerData,
                      SHttpRequest  *       pRequest,
                      SHttpResponse *       pResponse);

    int  ProcessMessage(const JsonNodePtr & node,
                        JsonNodePtr &       output,
                        SHttpHandlerData *  pHandlerData);

    int  ProcessHandshake(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessConnect(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessDisconnect(const JsonNodePtr &message, JsonNodePtr &output);

    int  ProcessSubscribe(const JsonNodePtr &   message,
                          JsonNodePtr &         output,
                          SHttpHandlerData *    pHandlerData);

    int  ProcessUnsubscribe(const JsonNodePtr & message,
                            JsonNodePtr &       output,
                            SHttpHandlerData *  pHandlerData);

    int  ProcessMetaMessage(const SString & channel,
                            const JsonNodePtr & message,
                            JsonNodePtr &       output,
                            SHttpHandlerData *  pHandlerData);

    int  ProcessPublish(const SString &         channel,
                        const JsonNodePtr &     message,
                        JsonNodePtr &           output,
                        SHttpHandlerData *      pHandlerData);

    bool AddSubscription(const SString &channel, const SString &clientId);
    bool RemoveSubscription(const SString &channel, const SString &clientId);

    bool AddClient(const SString &clientId, SHttpHandlerData *pHandlerData);
    SHttpHandlerData *GetClient(const SString &clientId);
    SHttpHandlerData *RemoveClient(const SString &clientId);

protected:
    //! Collection of channels
    typedef std::map<SString, SBayeuxChannel *>     ChannelMap;

    //! The connection to send data from for each client
    typedef std::map<SString, SHttpHandlerData *>   ChannelConnections;

    //! Clients connected to a channel
    typedef std::map<SString, SStringList *>        ChannelClients;

    //! The handler stage that is driving us all.
    SHttpHandlerStage *     pHandlerStage;

    //! Channels that are currently registered
    ChannelMap              channels;

    //! A list of channel subscriptions
    ChannelConnections      connections;

    //! List of clients for each channel
    ChannelClients          subscriptions;

    //! The boundary to be used bw multi part messages
    SString                 boundary;
};

#endif

