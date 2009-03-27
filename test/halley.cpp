
#include "eds/server.h"
#include "eds/connection.h"
#include "eds/stage.h"
#include "eds/fileiohelper.h"
#include "eds/http/request.h"
#include "eds/http/response.h"
#include "eds/http/readerstage.h"
#include "eds/http/handlerstage.h"
#include "eds/http/urlrouter.h"
#include "eds/http/filemodule.h"
#include "eds/http/bayeux/bayeuxmodule.h"
#include "eds/http/bayeux/channel.h"
#include "eds/http/contentmodule.h"
#include "eds/http/transfermodule.h"
#include "eds/http/writermodule.h"
#include "net/connhandler.h"
#include "net/connfactory.h"
#include "net/server.h"
#include "thread/thread.h"
#include <iostream>
using namespace std;

// we generate the content!!
class SMyModule : public SHttpModule
{
public:
    // Constructor
    SMyModule(SHttpModule *pNext) : SHttpModule(pNext) { }

    //! Called to handle input data from another module
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                             SHttpHandlerStage *    pStage, 
                             SBodyPart *            pBodyPart);
};

//! our bayeux connection handler
class MyConnHandler : public SConnHandler, public SChannelListener
{
public:
    MyConnHandler(SBayeuxChannel *pChannel, SBayeuxModule *pMod)
        : pTheChannel(pChannel), pModule(pMod), prompt(" Hello World >> ")
    {
        pTheChannel->SetChannelListener(this);
    }

    void HandleEvent(const JsonNodePtr &message, JsonNodePtr &output)
    {
        prompt = message->Get<SString>("prompt", "");
    }

protected:
    //! Handle connection in async mode
    virtual bool HandleConnection() { Start(); return false; }

    //! handles a custom connection
    virtual int Run()
    {
        cerr << "Accepting message from port: " << pServer->GetPort() << ", Socket: " << clientSocket << endl;
        while (!Stopped())
        {
            char buffer[1025];

            clientInput->getline(buffer, 1025);
            if (clientInput->bad() || clientInput->fail() || clientInput->eof())
                break ;

            JsonNodePtr value = JsonNodeFactory::StringNode(prompt + buffer);
            pModule->DeliverEvent(pTheChannel, value);
        }

        pServer->HandlerFinished(this);
        return 0;
    }

protected:
    //! The channel which is controlling it
    SBayeuxChannel * pTheChannel;

    //! Module thorough which events are dispatched
    SBayeuxModule *pModule;

    //! The prompt for this.
    std::string prompt;
};

class MyConnFactory : public SConnFactory
{
public:
    MyConnFactory(SBayeuxChannel *pChannel, SBayeuxModule *pMod)
        : pTheChannel(pChannel), pModule(pMod) { }

    virtual ~MyConnFactory() { }

    virtual SConnHandler *  NewHandler()  { return new MyConnHandler(pTheChannel, pModule); }
    virtual void            ReleaseHandler(SConnHandler * handler) { delete handler; }

protected:
    //! The channel which is controlling it
    SBayeuxChannel * pTheChannel;

    //! Module thorough which events are dispatched
    SBayeuxModule *pModule;
};

class MyBayeuxChannel : public virtual SBayeuxChannel, public virtual SServer
{
public:
    //! Constructor
    MyBayeuxChannel(SBayeuxModule *pMod, const std::string &name, int port) :
        SBayeuxChannel(name), SServer(port), pModule(pMod)
    {
        SetConnectionFactory(new MyConnFactory(this, pModule));
    }

protected:
    void HandleConnection(int clientSocket)
    {
        JsonNodePtr value = JsonNodeFactory::StringNode(" ===== Handling New Connection on Channel: " + Name());
        pModule->DeliverEvent(this, value);

        SServer::HandleConnection(clientSocket);

        value = JsonNodeFactory::StringNode(" ===== Connection Finished on Channel: " + Name());
        pModule->DeliverEvent(this, value);
    }

protected:
    //! The bayeux module through which events are dispatched
    SBayeuxModule * pModule;
};

// This is what drives the server and loads modules depending on how we
// want it.
int main(int argc, char *argv[])
{
    SHttpReaderStage    requestReader;
    SHttpHandlerStage   requestHandler;
    SFileIOHelper       fileHelper;
    SWriterModule       writerModule;
    // STransferModule     transferModule(&writerModule);
    SContentModule      contentModule(&writerModule);
    SBayeuxModule       bayeuxModule(&contentModule, "MyTestBoundary");
    SFileModule         rootFileModule(&contentModule, true);
    SMyModule           myModule(&contentModule);
    SFileModule         testModule(&contentModule, true);
    SUrlRouter          urlRouter(&myModule);
    SContainsUrlMatcher staticUrlMatch("/static/", SContainsUrlMatcher::PREFIX_MATCH, &rootFileModule);
    SContainsUrlMatcher testUrlMatch("/btest/", SContainsUrlMatcher::PREFIX_MATCH, &testModule);
    SContainsUrlMatcher dsUrlMatch("/bayeux/", SContainsUrlMatcher::PREFIX_MATCH, &bayeuxModule);

    testModule.AddDocRoot("/btest/", "/home/spanyam/personal/halley/test/");
    rootFileModule.AddDocRoot("/static/", "/");

    urlRouter.AddUrlMatch(&staticUrlMatch);
    urlRouter.AddUrlMatch(&testUrlMatch);
    urlRouter.AddUrlMatch(&dsUrlMatch);

    requestReader.SetHandlerStage(&requestHandler);
    requestHandler.SetRootModule(&urlRouter);

    int port = argc <= 1 ? 80 : atoi(argv[1]);
    SEvServer pServer(port, &requestReader);

    pServer.SetStage("RequestReader", &requestReader);
    pServer.SetStage("RequestHandler", &requestHandler);

    for (int i = 0;i < 5;i++)
    {
        SStringStream sstr;
        sstr << "/bayeux/channel" << (i + 1);
        int port = 1010 + (i * 10);
        MyBayeuxChannel *pChannel = new MyBayeuxChannel(&bayeuxModule, sstr.str(), port);
        cerr << "Starting bayeux channel: " << sstr.str() << " on port: " << port << endl;
        bayeuxModule.RegisterChannel(pChannel);
        SThread *pChannelThread = new SThread(pChannel);
        pChannelThread->Start();
    }

    cerr << "Server Started on port: " << port << "..." << endl;
    pServer.Start();
    cerr << "Server Finished..." << endl;

    return 0;
}

//! Called to handle input data from another module
void SMyModule::ProcessInput(SHttpHandlerData *     pHandlerData,
                             SHttpHandlerStage *    pStage,
                             SBodyPart *            pBodyPart)
{
    SHttpRequest *pRequest      = pHandlerData->Request();
    SHttpResponse *pResponse    = pRequest->Response();
    SHeaderTable &reqHeaders    = pRequest->Headers();
    SString title;
    SString body;
    SString bgcolor="#ffffff";
    SString links =
            "<p><a href='/red'>red</a> "
            "<br><a href='/blue'>blue</a> "
            "<br><a href='/form'>form</a> "
            "<br><a href='/auth'>authentication example</a> [use <b>adp</b> as username and <b>gmbh</b> as password"
            "<br><a href='/header'>show some HTTP header details</a> "
            "<br><a href='/btest/'>Bayeux Test</a> "
            ;

    if(pRequest->Resource() == "/") {
        title = "Web Server Example";
        body    = "I wonder what you're going to click"    + links;
    }
    else if (pRequest->Resource() == "/red") {
        bgcolor = "#ff4444";
        title     = "You chose red";
        body        = "<h1>Red</h1>" + links;
    }
    else if (pRequest->Resource() == "/blue") {

        bgcolor = "#4444ff";
        title     = "You chose blue";
        body        = "<h1>Blue</h1>" + links;
    }
    else if (pRequest->Resource() == "/form") {
        title    = "Fill a form";
        body    = "<h1>Fill a form</h1>";
        body    += "<form action='/form'>"
                            "<table>"
                            "<tr><td>Field 1</td><td><input name=field_1></td></tr>"
                            "<tr><td>Field 2</td><td><input name=field_2></td></tr>"
                            "<tr><td>Field 3</td><td><input name=field_3></td></tr>"
                            "</table>"
                            "<input type=submit></form>";
        body += "<hr>" + links;
    }
    else if (pRequest->Resource() == "/header") {
        title    = "HTTP Headers";
        body    = "<h1> Your HTTP Headers</h1>";
        for (std::map<SString, SString>::const_iterator i = reqHeaders.FirstHeader();
                 i != reqHeaders.LastHeader();
                 i++)
        {
                body += "<br>" + i->first + " = " + i->second;
        }

        body += "<hr>" + links;
    }
    else {
        pResponse->SetStatus(404, "Not Found");
        title   = "Wrong URL";
        body    = "<h1>Wrong URL</h1>";
        body    += "Path is : &gt;" + pRequest->Resource() + "&lt;"; 
    }

    // pResponse->SetHeader("Connection", "Close");
    SBodyPart *part = pResponse->NewBodyPart(0, pNextModule);
    part->SetBody("<html><head><title>");
    part->AppendToBody(title);
    part->AppendToBody("</title></head><body bgcolor='" + bgcolor + "'>");
    part->AppendToBody(body);

    std::cerr << " === MyModule: Sending to next module" << std::endl;
    pStage->OutputToModule(pHandlerData->pConnection, pNextModule, part);
    pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                           pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED, pNextModule));
}

