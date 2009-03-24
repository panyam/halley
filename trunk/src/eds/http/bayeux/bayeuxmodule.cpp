//*****************************************************************************
/*!
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

const char *FIELD_CHANNEL               = "channel";
const char *FIELD_VERSION               = "version";
const char *FIELD_MINVERSION            = "minimumVersion";
const char *FIELD_CONNTYPE              = "connectionType";
const char *FIELD_SUPPORTED_CONNTYPES   = "supportedConnectionTypes";
const char *FIELD_CLIENTID              = "clientId";
const char *FIELD_ADVICE                = "advice";
const char *FIELD_ID                    = "id";
const char *FIELD_EXT                   = "ext";
const char *FIELD_SUCCESSFUL            = "successful";
const char *FIELD_AUTHSUCCESSFUL        = "authSuccessful";
const char *FIELD_SUBSCRIPTION          = "subscription";

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
bool SBayeuxModule::AddSubscription(const SString &channel, SConnection *pConnection)
{
    ChannelSubscription::iterator iter  = subscriptions.find(channel);
    SConnectionList *pConnList          = NULL;
    if (iter == subscriptions.end())
    {
        pConnList = new SConnectionList();
        subscriptions.insert(std::pair<SString, SConnectionList *>(channel, pConnList));
    }
    else
    {
        pConnList = iter->second;
    }

    if (find(pConnList->begin(), pConnList->end(), pConnection) != pConnList->end())
        return false;

    pConnList->push_back(pConnection);

    return true;
}

bool SBayeuxModule::RemoveSubscription(const SString &channel, SConnection *pConnection)
{
    ChannelSubscription::iterator iter  = subscriptions.find(channel);
    if (iter == subscriptions.end())
        return false;

    SConnectionList *pConnList = iter->second;

    SConnectionList::iterator iter2 = find(pConnList->begin(), pConnList->end(), pConnection);

    if (iter2 == pConnList->end())
        return false;

    pConnList->erase(iter2);

    return true;
}

//! Delivers an event to all subscribers of a channel
void SBayeuxModule::DeliverEvent(SBayeuxChannel *pChannel, JsonNodePtr &value)
{
    // do nothing if no handler stage available
    if (pHandlerStage == NULL) return ;

    ChannelSubscription::iterator iter  = subscriptions.find(pChannel->Name());

    if (iter == subscriptions.end())
        return ;

    SStringStream msgstream;
    DefaultJsonFormatter formatter;
    formatter.Format(msgstream, value);

    SString             msgbody(msgstream.str());
    SConnectionList *   pConnList(iter->second);
    for (SConnectionList::iterator iter = pConnList->begin();
                    iter != pConnList->end(); ++iter)
    {
        SConnection *       pConnection = *iter;
        SHttpHandlerData *  pData       = (SHttpHandlerData *)pConnection->GetStageData(pHandlerStage);
        SHttpRequest *      pRequest    = pData->Request();
        SHttpResponse *     pResponse   = pRequest->Response();
        SBodyPart *         pNewPart    = pResponse->NewBodyPart();
        pNewPart->SetBody(msgbody);
        pHandlerStage->OutputToModule(pData->pConnection, pNextModule, pNewPart);
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
    SConnection *pConnection            = pHandlerData->pConnection;
    SHttpRequest *pRequest              = pHandlerData->Request();
    SHttpResponse *pResponse            = pRequest->Response();
    SBodyPart *pContent                 = pRequest->ContentBody();
    SHeaderTable & respHeaders(pResponse->Headers());

    JsonNodePtr output      = JsonNodeFactory::StringNode("<html><title>Error</title><body>Invalid bayeux message</body></html>");
    int         result      = -1;

    if (pContent != NULL)
    {
        const SCharVector & reqBody         = pContent->Body();
        // parse the list of messages
        DefaultJsonInputStream<SCharVector::const_iterator> instream(reqBody.begin(),
                                                                     reqBody.end());
        DefaultJsonBuilder jbuilder;
        JsonNodePtr messages    = jbuilder.Build(&instream);

        // see if it is a single message or a list of messages:
        if (messages->Type() == JNT_LIST)
        {
            for (int i = 0, count = messages->Size();i < count;i++)
            {
                result = ProcessMessage(messages->Get(i), output, pConnection);
                if (result < 0) break;
            }
        }
        else if (messages->Type() == JNT_OBJECT)
        {
            result = ProcessMessage(messages, output, pConnection);
        }
    }

    if (result <= 0)
    {
        int         statCode = 200;
        std::string statMessage("OK");
        if (result < 0)
        {
            // assert("Type MUST be a string if result is -ve" &&
            //            (output->Type() == JNT_STRING));

            statCode    = 500;
            statMessage = "Invalid Message";
        }

        // invalid type
        SStringStream msgstream;
        DefaultJsonFormatter formatter;
        formatter.Format(msgstream, output);

        SBodyPart * part        = pResponse->NewBodyPart();
        SString     msgbody     = msgstream.str();
        respHeaders.SetIntHeader("Content-Length", msgbody.size());
        respHeaders.SetHeader("Content-Type", "text/html");
        part->SetBody(msgbody);

        pResponse->SetStatus(statCode, statMessage);
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
    }
}

//! Processes a message and appends the result (json) to the output list.
// Returns:
//  -1 on error,
//  0 on success and to return a response immediately
//  1 on success but to make 
int SBayeuxModule::ProcessMessage(const JsonNodePtr &   message,
                                  JsonNodePtr &         output,
                                  SConnection *         pConnection)
{
    SString channel = message->Get<SString>("channel", "");
    if (channel == "")
    {
        output = JsonNodeFactory::StringNode("Channel name missing");
        return -1;
    }

    if (!output || output->Type() != JNT_LIST)
        output = JsonNodeFactory::ListNode();

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
            return ProcessSubscribe(message, output, pConnection);
        }
        else if (channel == "/meta/unsubscribe")
        {
            return ProcessUnsubscribe(message, output, pConnection);
        }
        else
        {
            // other meta messages
            return ProcessMetaMessage(channel, message, output, pConnection);
        }
    }
    else 
    {
        // message is for a channel
        return ProcessPublish(channel, message, output, pConnection);
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
    SString uuid_string(uuid_string);
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
                                     SConnection *      pConnection)
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

    AddSubscription(subscription, pConnection);

    // TODO: merge subscriptions from the same browser into 1.

    return 0;
}

int SBayeuxModule::ProcessUnsubscribe(const JsonNodePtr &  message,
                                       JsonNodePtr &        output,
                                       SConnection *        pConnection)
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
    RemoveSubscription(subscription, pConnection);

    return 0;
}

int SBayeuxModule::ProcessPublish(const SString &       channel,
                                  const JsonNodePtr &   message,
                                  JsonNodePtr &         output,
                                  SConnection *         pConnection)
{
    // do all the stuff here
    output = JsonNodeFactory::StringNode("No handler for publish request found.");
    return -1;
}

int SBayeuxModule::ProcessMetaMessage(const SString &       channel,
                                      const JsonNodePtr &   message,
                                      JsonNodePtr &         output,
                                      SConnection *         pConnection)
{
    output = JsonNodeFactory::StringNode("Invalid meta channel");
    return -1;
}

