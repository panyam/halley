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
}

//! Processes a message and appends the result (json) to the output list.
bool SBayeuxModule::ProcessMessage(const JsonNodePtr &message, JsonNodePtr &output)
{
    if (!output || output->Type() != JNT_LIST)
        output = JsonNodeFactory::ListNode();

    return true;
}

