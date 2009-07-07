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
 *  \file   bayeuxmodule.cpp
 *
 *  \brief  A module for handling bayeux connections.  Hello Comet!!!
 *
 *  \version
 *      - S Panyam      23/03/2009
 *        Created
 *
 *****************************************************************************/

#include "bayeuxmodule.h"
#include "channel.h"
#include "../handlerstage.h"
#include "../request.h"
#include "../response.h"
#include "../../connection.h"
#include "json/json.h"
#include "json/tokenizer.h"
#include <uuid/uuid.h>

const SString EMPTY("");
const SString FIELD_CHANNEL             = "channel";
const SString FIELD_DATA                = "data";
const SString FIELD_VERSION             = "version";
const SString FIELD_FIRSTCONN           = "firstconn";
const SString FIELD_MINVERSION          = "minimumVersion";
const SString FIELD_CONNTYPE            = "connectionType";
const SString FIELD_SUPPORTED_CONNTYPES = "supportedConnectionTypes";
const SString FIELD_CLIENTID            = "clientId";
const SString FIELD_ADVICE              = "advice";
const SString FIELD_ID                  = "id";
const SString FIELD_EXT                 = "ext";
const SString FIELD_SUCCESSFUL          = "successful";
const SString FIELD_AUTHSUCCESSFUL      = "authSuccessful";
const SString FIELD_SUBSCRIPTION        = "subscription";

//! Registers a channel
bool SBayeuxModule::RegisterChannel(SBayeuxChannel *pChannel, bool replace)
{
    ChannelMap::iterator iter = channels.find(pChannel->Name());
    if (iter != channels.end())
    {
        if (!replace) return false;

        // TODO: should unregister be called instead?
        channels.erase(iter);
    }
    channels.insert(std::pair<SString, SBayeuxChannel *>(pChannel->Name(), pChannel));
    return true;
}

//! Get a channel by name
SBayeuxChannel *SBayeuxModule::GetChannel(const SString &name)
{
    ChannelMap::iterator iter = channels.find(name);
    if (iter == channels.end())
        return NULL;
    else 
        return iter->second;
}

//! Removes a channel by name
bool SBayeuxModule::UnregisterChannel(const SString &name)
{
    ChannelMap::iterator iter = channels.find(name);
    if (iter == channels.end())
        return false;

    channels.erase(iter);
    return true;
}

//! Removes a channel
bool SBayeuxModule::UnregisterChannel(const SBayeuxChannel *pChannel)
{
    return UnregisterChannel(pChannel->Name());
}

//! Adds a new connection to the subscription list
// TODO: thread safety
bool SBayeuxModule::AddSubscription(const SString &channel, const SString &clientId)
{
    ChannelClients::iterator    iter        = subscriptions.find(channel);
    SStringList *               pClientList = NULL;
    if (iter == subscriptions.end())
    {
        pClientList = new SStringList();
        subscriptions.insert(std::pair<SString, SStringList *>(channel, pClientList));
    }
    else
    {
        pClientList = iter->second;
    }

    if (find(pClientList->begin(), pClientList->end(), clientId) == pClientList->end())
    {
        pClientList->push_back(clientId);
        return true;
    }
    return false;
}

bool SBayeuxModule::RemoveSubscription(const SString &channel, const SString &clientId)
{
    ChannelClients::iterator iter = subscriptions.find(channel);
    if (iter == subscriptions.end())
    {
        SStringList *pClientList = iter->second;
        if (pClientList != NULL)
        {
            SStringList::iterator iter2 = find(pClientList->begin(), pClientList->end(), clientId);

            if (iter2 != pClientList->end())
            {
                pClientList->erase(iter2);
                return true;
            }
        }
    }
    return false;
}

bool SBayeuxModule::AddClient(const SString &clientId, SHttpHandlerData *pHandlerData)
{
    ChannelConnections::iterator iter = connections.find(clientId);
    if (iter != connections.end())
        return false;

    connections.insert(std::pair<SString, SHttpHandlerData *>(clientId, pHandlerData));

    return true;
}

SHttpHandlerData *SBayeuxModule::GetClient(const SString &clientId)
{
    ChannelConnections::iterator iter = connections.find(clientId);
    if (iter == connections.end())
        return NULL;
    else
        return iter->second;
}

SHttpHandlerData *SBayeuxModule::RemoveClient(const SString &clientId)
{
    ChannelConnections::iterator iter = connections.find(clientId);
    if (iter == connections.end())
        return NULL;

    connections.erase(iter);
    return iter->second;
}

//! Delivers an event to all subscribers of a channel
void SBayeuxModule::DeliverEvent(const SBayeuxChannel *pChannel, const JsonNodePtr &value)
{
    // do nothing if no handler stage available
    if (pHandlerStage == NULL || pChannel == NULL) return ;

    ChannelClients::iterator iter  = subscriptions.find(pChannel->Name());

    if (iter == subscriptions.end())
        return ;

    JsonNodePtr realValue = JsonNodeFactory::ObjectNode();
    realValue->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode(pChannel->Name()));
    realValue->Set(FIELD_DATA, value);

    SStringStream msgstream;
    DefaultJsonFormatter formatter;
    formatter.Format(msgstream, realValue);
    SString msgbody(msgstream.str());

    SStringList *pClientList = iter->second;
    for (SStringList::iterator iter = pClientList->begin();iter != pClientList->end();++iter)
    {
        SHttpHandlerData *pHandlerData = GetClient(*iter);
        if (pHandlerData != NULL)
        {
            SHttpRequest *      pRequest    = pHandlerData->Request();
            SHttpResponse *     pResponse   = pRequest->Response();
            SBodyPart *         pBodyPart   = pResponse->NewBodyPart();
            pBodyPart->SetBody(msgbody);
            pHandlerStage->OutputToModule(pHandlerData->pConnection, pNextModule, pBodyPart);
        }
    }
}

//! returns true if a character is a hyphen
bool equalsHyphen(const char &ch) { return ch == '-'; }
bool notAlpha(const char &ch) { return !isalnum(ch); }

//! Called to handle subscriptions, unsubscriptions and callbacks
//
// Refer to http://svn.cometd.org/trunk/bayeux/bayeux.html for more details
// on the Bayeux protocol.
//
// Another things is that this module can be written in an external module,
// no reason why it should be here, but I still have not decided on the FFI
// for halley so will defer writing foreign language bindings till Ive
// tried out a few scenarious.
//
void SBayeuxModule::ProcessInput(SHttpHandlerData *     pHandlerData,
                                 SHttpHandlerStage *    pStage, 
                                 SBodyPart *            pBodyPart)
{
    this->pHandlerStage                 = pStage;
    SHttpRequest *pRequest              = pHandlerData->Request();
    SHttpResponse *pResponse            = pRequest->Response();
    SBodyPart *pContent                 = pRequest->ContentBody();

    JsonNodePtr output      = JsonNodeFactory::StringNode("<html><title>Error</title><body>Invalid bayeux message</body></html>");
    int         result      = -1;

    if (pContent != NULL)
    {
        const SCharVector & reqBody         = pContent->Body();
        // parse the list of messages
        DefaultJsonInputStream<SCharVector::const_iterator> instream(reqBody.begin(),
                                                                     reqBody.end());
        cout << "Bayeux Body: ";
        copy(reqBody.begin(), reqBody.end(), std::ostreambuf_iterator<char>(cout));
        cout << endl;

        DefaultJsonBuilder jbuilder;
        JsonNodePtr messages    = jbuilder.Build(&instream);

        // see if it is a single message or a list of messages:
        if (messages->Type() == JNT_LIST)
        {
            for (int i = 0, count = messages->Size();i < count;i++)
            {
                result = ProcessMessage(messages->Get(i), output, pHandlerData);
                if (result < 0) break;
            }
        }
        else if (messages->Type() == JNT_OBJECT)
        {
            result  = ProcessMessage(messages, output, pHandlerData);
        }
    }

    SendResponse(result, output, pStage, pHandlerData, pResponse);
}

void SBayeuxModule::SendResponse(int                result,
                                 const JsonNodePtr &output,
                                 SHttpHandlerStage *pStage,
                                 SHttpHandlerData * pHandlerData,
                                 SHttpResponse *    pResponse)
{
    SHeaderTable & respHeaders(pResponse->Headers());

    // set response code and message
    if (result < 0)
    {
        pResponse->SetStatus(500, "Invalid Message");
    }

    if (result <= 0)
    {
        SStringStream msgstream;
        DefaultJsonFormatter formatter;
        formatter.Format(msgstream, output);

        SString     msgbody     = msgstream.str();
        respHeaders.SetIntHeader("Content-Length", msgbody.size());
        respHeaders.SetHeader("Content-Type", "text/html");

        SBodyPart * part        = pResponse->NewBodyPart();
        part->SetBody(msgbody);

        pStage->OutputToModule(pHandlerData->pConnection, pNextModule, part);
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                               pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED,
                                                      pNextModule));
    }
    else
    {
        respHeaders.SetHeader("Content-Type",
                              "multipart/x-mixed-replace;boundary=\"" +
                                    boundary + SString("\""));

        SString channel = output->Get(FIELD_SUBSCRIPTION, EMPTY);

        // and deliver the message 
        JsonNodePtr realValue = JsonNodeFactory::ObjectNode();
        realValue->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode(channel));
        realValue->Set(FIELD_DATA, output);

        SStringStream           msgstream;
        DefaultJsonFormatter    formatter;
        formatter.Format(msgstream, realValue);

        // open a boundary body part 
        SBodyPart *pBodyPart = pResponse->NewBodyPart(SBodyPart::BP_OPEN_SUB_MESSAGE);
        pBodyPart->SetBody(boundary);
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule, pBodyPart);

        // send the first message!
        pBodyPart = pResponse->NewBodyPart();
        pBodyPart->SetBody(msgstream.str());
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule, pBodyPart);
    }
}

//! Processes a message and appends the result (json) to the output list.
// Returns:
//  -1 on error,
//  0 on success and to return a response immediately
//  1 on success but to make 
int SBayeuxModule::ProcessMessage(const JsonNodePtr &   message,
                                  JsonNodePtr &         output,
                                  SHttpHandlerData *    pHandlerData)
{
    SString channel = message->Get<SString>("channel", "");
    if (channel == "")
    {
        output = JsonNodeFactory::StringNode("Channel name missing");
        return -1;
    }

    // see what kind of message it is
    if (strncmp(channel.c_str(), "/meta/", 6) == 0)
    {
        if (channel == "/meta/handshake")
        {
            return ProcessHandshake(message, output);
        }
        else if (channel == "/meta/connect")
        {
            return ProcessConnect(message, output);
        }
        else if (channel == "/meta/disconnect")
        {
            return ProcessDisconnect(message, output);
        }
        else if (channel == "/meta/subscribe")
        {
            return ProcessSubscribe(message, output, pHandlerData);
        }
        else if (channel == "/meta/unsubscribe")
        {
            return ProcessUnsubscribe(message, output, pHandlerData);
        }
        else
        {
            // other meta messages
            return ProcessMetaMessage(channel, message, output, pHandlerData);
        }
    }
    else 
    {
        // message is for a channel
        return ProcessPublish(channel, message, output, pHandlerData);
    }
}

//! Processes a handshake.  Default is to simply return a true to accept
// everything.  Override this to do multi-level handshakes and
// authentication etc.
int SBayeuxModule::ProcessHandshake(const JsonNodePtr &message, JsonNodePtr &output)
{
    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/handshake"));
    output->Set(FIELD_VERSION, JsonNodeFactory::StringNode("1.0"));
    output->Set(FIELD_MINVERSION, JsonNodeFactory::StringNode("1.0"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_AUTHSUCCESSFUL, JsonNodeFactory::BoolNode(true));

    JsonNodePtr connTypes = JsonNodeFactory::ListNode();
    connTypes->Add(JsonNodeFactory::StringNode("long-polling"));
    connTypes->Add(JsonNodeFactory::StringNode("callback-polling"));
    connTypes->Add(JsonNodeFactory::StringNode("iframe"));
    output->Set(FIELD_SUPPORTED_CONNTYPES, connTypes);

    // calculate the uuid
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[40];
    uuid_unparse(uuid, uuid_str);

    // strip the "-"s from the uuid_str
    SString uuid_string(uuid_str);
    uuid_string.erase(std::remove_if(uuid_string.begin(), uuid_string.end(), notAlpha),
                      uuid_string.end());

    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(uuid_string));

    // TODO: do something with the client id like registering it etc
    //
    // Override this, and extend to do things like handling auth and so on.

    return 0;
}

int SBayeuxModule::ProcessConnect(const JsonNodePtr &message, JsonNodePtr &output)
{
    SString clientId(message->Get<SString>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return -1;
    }

    SString connectionType(message->Get<SString>(FIELD_CONNTYPE, ""));
    if (connectionType == "")
    {
        output = JsonNodeFactory::StringNode("connectionType missing.");
        return -1;
    }

    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));

    // TODO: register this client and do things like handle "timeouts" with
    // handshakes and so on...

    return 0;
}

int SBayeuxModule::ProcessDisconnect(const JsonNodePtr &message, JsonNodePtr &output)
{
    SString clientId(message->Get<SString>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return -1;
    }

    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));

    // TODO: register this client and do things like handle "timeouts" with
    // handshakes and so on...

    return 0;
}

int SBayeuxModule::ProcessSubscribe(const JsonNodePtr & message,
                                     JsonNodePtr &      output,
                                     SHttpHandlerData * pHandlerData)
{
    SString clientId(message->Get<SString>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return -1;
    }

    SString subscription(message->Get<SString>(FIELD_SUBSCRIPTION, ""));
    if (subscription == "")
    {
        output = JsonNodeFactory::StringNode("subscription missing.");
        return -1;
    }

    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));
    output->Set(FIELD_SUBSCRIPTION, JsonNodeFactory::StringNode(subscription));

    bool firstConn = AddClient(clientId, pHandlerData);
    bool subscribed = AddSubscription(subscription, clientId);

    output->Set(FIELD_FIRSTCONN, JsonNodeFactory::BoolNode(firstConn));
    output->Set("subscribed", JsonNodeFactory::BoolNode(subscribed));

    // if first connection then dont close this connection - this is THE
    // long living one!!!
    return firstConn ? 1 : 0;
}

int SBayeuxModule::ProcessUnsubscribe(const JsonNodePtr &   message,
                                       JsonNodePtr &        output,
                                       SHttpHandlerData *   pHandlerData)
{
    SString clientId(message->Get<SString>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return -1;
    }

    SString subscription(message->Get<SString>(FIELD_SUBSCRIPTION, ""));
    if (subscription == "")
    {
        output = JsonNodeFactory::StringNode("subscription missing.");
        return -1;
    }

    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));
    output->Set(FIELD_SUBSCRIPTION, JsonNodeFactory::StringNode(subscription));

    // TODO: again handle all the "real" stuff below
    RemoveSubscription(subscription, clientId);

    return 0;
}

int SBayeuxModule::ProcessPublish(const SString &       channel,
                                  const JsonNodePtr &   message,
                                  JsonNodePtr &         output,
                                  SHttpHandlerData *    pHandlerData)
{
    // do all the stuff here
    output = JsonNodeFactory::StringNode("OK");
    SBayeuxChannel *pChannel = GetChannel(channel);
    pChannel->HandleEvent(message, output);
    return 0;
}

int SBayeuxModule::ProcessMetaMessage(const SString &       channel,
                                      const JsonNodePtr &   message,
                                      JsonNodePtr &         output,
                                      SHttpHandlerData *    pHandlerData)
{
    output = JsonNodeFactory::StringNode("Invalid meta channel");
    return -1;
}

