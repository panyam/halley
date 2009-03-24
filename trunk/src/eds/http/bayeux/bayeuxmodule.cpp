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
#include "../handlerstage.h"
#include "../request.h"
#include "../response.h"
#include "json/json.h"
#include "json/tokenizer.h"
#include <uuid/uuid.h>

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
    else if (strncmp(channel.c_str(), "/meta/", 6) == 0)
    {
        return ProcessMetaMessage(message, output);
    }
    else 
    {
        return ProcessPublish(message, output);
    }
}

bool SBayeuxModule::ProcessHandshake(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessConnect(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessDisconnect(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessSubscribe(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessUnsubscribe(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessMetaMessage(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

bool SBayeuxModule::ProcessPublish(JsonNodePtr message, JsonNodePtr &output)
{
    return true;
}

