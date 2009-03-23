
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
#include "eds/http/bayeuxmodule.h"
#include "eds/http/contentmodule.h"
#include "eds/http/transfermodule.h"
#include "eds/http/writermodule.h"
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

//! A data source that sends data to the data source module
class SDataSource
{
public:
    //! Creates a data source
    SDataSource(const std::string &name, SHttpHandlerStage *pStage)
        : name(n), pHandlerStage(pStage) { }

    void AddDSModule(SBayeuxModule *pModule)
    {
        if (find(bayeuxModules.begin(), bayeuxModules.end(), pModule) == bayeuxModules.end())
        {
            bayeuxModules.push_back(pModule);
        }
    }

public:
    //! Name of the data source
    std::string                     name;

    //! The handler stage which will be notified when new data has arrived
    SHttpHandlerStage *             pHandlerStage;

    //! The data source modules to which this data source will send data
    // when new data is ready.
    std::list<SBayeuxModule *>  bayeuxModules;
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
    SBayeuxModule       bayeuxModule(&contentModule);
    SFileModule         rootFileModule(&contentModule, true);
    SMyModule           myModule(&contentModule);
    SUrlRouter          urlRouter(&myModule);
    SContainsUrlMatcher staticUrlMatch("/static/", SContainsUrlMatcher::PREFIX_MATCH, &rootFileModule);
    SContainsUrlMatcher dsUrlMatch("/ds/", SContainsUrlMatcher::PREFIX_MATCH, &dsmModule);

    rootFileModule.AddDocRoot("/static/", "/");

    urlRouter.AddUrlMatch(&staticUrlMatch);
    urlRouter.AddUrlMatch(&dsUrlMatch);

    requestReader.SetHandlerStage(&requestHandler);
    requestHandler.SetRootModule(&urlRouter);

    int port = argc <= 1 ? 80 : atoi(argv[1]);
    SEvServer pServer(port, &requestReader);

    pServer.SetStage("RequestReader", &requestReader);
    pServer.SetStage("RequestHandler", &requestHandler);

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
    std::string title;
    std::string body;
    std::string bgcolor="#ffffff";
    std::string links =
            "<p><a href='/red'>red</a> "
            "<br><a href='/blue'>blue</a> "
            "<br><a href='/form'>form</a> "
            "<br><a href='/auth'>authentication example</a> [use <b>adp</b> as username and <b>gmbh</b> as password"
            "<br><a href='/header'>show some HTTP header details</a> "
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
        for (std::map<std::string, std::string>::const_iterator i = reqHeaders.FirstHeader();
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

