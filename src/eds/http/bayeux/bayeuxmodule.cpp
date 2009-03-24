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
    channels.insert(std::pair<std::string, SBayeuxChannel *>(pChannel->Name(), pChannel));
    return true;
}

//! Removes a channel by name
bool SBayeuxModule::UnregisterChannel(const std::string &name)
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
    SHttpRequest *pRequest              = pHandlerData->Request();
    SHttpResponse *pResponse            = pRequest->Response();
    SHeaderTable & respHeaders(pResponse->Headers());
    SBodyPart *pContent                 = pRequest->ContentBody();
    const std::vector<char> &reqBody    = pContent->Body();


    // parse the list of messages
    DefaultJsonInputStream<std::vector<char>::const_iterator> instream(reqBody.begin(),
                                                                       reqBody.end());
    DefaultJsonBuilder jbuilder;
    JsonNodePtr messages = jbuilder.Build(&instream);

    JsonNodePtr output;

    // see if it is a single message or a list of messages:
    if (messages->Type() == JNT_LIST)
    {
        for (int i = 0, count = messages->Size();i < count;i++)
        {
            if ( ! ProcessMessage(messages->Get(i), output))
                break ;
        }
    }
    else if (messages->Type() == JNT_OBJECT)
    {
        ProcessMessage(messages, output);
    }
    else
    {
        // invalid type
        pResponse->SetStatus(500, "Invalid bayeux message.");
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                               pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED, pNextModule));
    }

    if (output->Type() == JNT_STRING)
    {
        // invalid type
        SBodyPart * part        = pResponse->NewBodyPart();
        std::string errormsg    = output->Value<std::string>();
        respHeaders.SetIntHeader("Content-Length", errormsg.size());
        respHeaders.SetHeader("Content-Type", "text/text");
        part->SetBody(errormsg);

        pResponse->SetStatus(500, "Invalid bayeux message.");
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule, part);
        pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                               pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED, pNextModule));
    }
}

//! Processes a message and appends the result (json) to the output list.
bool SBayeuxModule::ProcessMessage(const JsonNodePtr &message, JsonNodePtr &output)
{
    std::string channel = message->Get<std::string>("channel", "");
    if (channel == "")
    {
        output = JsonNodeFactory::StringNode("Channel name missing");
        return false;
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
            return ProcessSubscribe(message, output);
        }
        else if (channel == "/meta/unsubscribe")
        {
            return ProcessUnsubscribe(message, output);
        }
        else
        {
            // other meta messages
            return ProcessMetaMessage(channel, message, output);
        }
    }
    else 
    {
        // message is for a channel
        return ProcessPublish(channel, message, output);
    }
}

//! Processes a handshake.  Default is to simply return a true to accept
// everything.  Override this to do multi-level handshakes and
// authentication etc.
bool SBayeuxModule::ProcessHandshake(const JsonNodePtr &message, JsonNodePtr &output)
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
    std::string uuid_string(uuid_string);
    uuid_string.erase(std::remove_if(uuid_string.begin(), uuid_string.end(), notAlpha), uuid_string.end());

    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(uuid_string));

    // TODO: do something with the client id like registering it etc
    //
    // Override this, and extend to do things like handling auth and so on.

    return true;
}

bool SBayeuxModule::ProcessConnect(const JsonNodePtr &message, JsonNodePtr &output)
{
    std::string clientId(message->Get<std::string>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return false;
    }

    std::string connectionType(message->Get<std::string>(FIELD_CONNTYPE, ""));
    if (connectionType == "")
    {
        output = JsonNodeFactory::StringNode("connectionType missing.");
        return false;
    }

    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));

    // TODO: register this client and do things like handle "timeouts" with
    // handshakes and so on...

    return true;
}

bool SBayeuxModule::ProcessDisconnect(const JsonNodePtr &message, JsonNodePtr &output)
{
    std::string clientId(message->Get<std::string>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return false;
    }

    output = JsonNodeFactory::ObjectNode();
    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));

    // TODO: register this client and do things like handle "timeouts" with
    // handshakes and so on...

    return true;
}

bool SBayeuxModule::ProcessSubscribe(const JsonNodePtr &message, JsonNodePtr &output)
{
    std::string clientId(message->Get<std::string>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return false;
    }

    std::string subscription(message->Get<std::string>(FIELD_SUBSCRIPTION, ""));
    if (subscription == "")
    {
        output = JsonNodeFactory::StringNode("subscription missing.");
        return false;
    }

    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));
    output->Set(FIELD_SUBSCRIPTION, JsonNodeFactory::StringNode(subscription));

    // TODO: again handle all the "real" stuff below

    return true;
}

bool SBayeuxModule::ProcessUnsubscribe(const JsonNodePtr &message, JsonNodePtr &output)
{
    std::string clientId(message->Get<std::string>(FIELD_CLIENTID, ""));
    if (clientId == "")
    {
        output = JsonNodeFactory::StringNode("Client ID missing.");
        return false;
    }

    std::string subscription(message->Get<std::string>(FIELD_SUBSCRIPTION, ""));
    if (subscription == "")
    {
        output = JsonNodeFactory::StringNode("subscription missing.");
        return false;
    }

    output->Set(FIELD_CHANNEL, JsonNodeFactory::StringNode("/meta/connect"));
    output->Set(FIELD_SUCCESSFUL, JsonNodeFactory::BoolNode(true));
    output->Set(FIELD_CLIENTID, JsonNodeFactory::StringNode(clientId));
    output->Set(FIELD_SUBSCRIPTION, JsonNodeFactory::StringNode(subscription));

    // TODO: again handle all the "real" stuff below

    return true;
}

bool SBayeuxModule::ProcessPublish(const std::string &channel, const JsonNodePtr &message, JsonNodePtr &output)
{
    // do all the stuff here
    output = JsonNodeFactory::StringNode("No handler for publish request found.");
    return false;
}

bool SBayeuxModule::ProcessMetaMessage(const std::string &channel, const JsonNodePtr &message, JsonNodePtr &output)
{
    output = JsonNodeFactory::StringNode("Invalid meta channel");
    return false;
}

