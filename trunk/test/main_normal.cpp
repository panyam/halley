
#include "net/server.h"
#include "net/connpool.h"
#include "net/http/http.h"
#include <iostream>
using namespace std;

class MyHandler : public SHttpRequestHandler
{
public:
    MyHandler()
    {
        cerr << "Creating request handler..." << endl;
    }

    virtual ~MyHandler()
    {
        cerr << "Destroying request handler..." << endl;
    }

    virtual bool ProcessRequest(SHttpRequest &request, SHttpResponse &response);
};

class MyConnPool : public SConnPool<SHttpConnHandler>
{
public:
    virtual SHttpConnHandler *NewHandler()
    {
        SHttpConnHandler *pHandler = SConnPool<SHttpConnHandler>::NewHandler();
        pHandler->RegisterRequestHandler(new MyHandler(), REQUEST_HANDLER);
        return pHandler;
    }

    virtual void ReleaseHandler(SConnHandler *pHandler)
    {
        SHttpConnHandler *pHttpHandler = dynamic_cast<SHttpConnHandler *>(pHandler);
        if (pHttpHandler != NULL)
        {
            SHttpRequestHandler *pReqHandler = pHttpHandler->UnRegisterRequestHandler(REQUEST_HANDLER);
            delete pReqHandler;
        }
        SConnPool<SHttpConnHandler>::ReleaseHandler(pHandler);
    }
};

int main(int argc, char *argv[])
{
    SServer *pServer = new SServer(argc <= 1 ? 80 : atoi(argv[1]));

    pServer->SetConnectionFactory(new MyConnPool());

    cerr << "Server Started ..." << endl;
    pServer->Start();
    cerr << "Server Finished..." << endl;

    return 0;
}

bool MyHandler::ProcessRequest(SHttpRequest &request, SHttpResponse &response)
{
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

  if(request.Resource() == "/") {

    title = "Web Server Example";
    body  = "I wonder what you're going to click"  + links;
  }
  else if (request.Resource() == "/red") {

    bgcolor = "#ff4444";
    title   = "You chose red";
    body    = "<h1>Red</h1>" + links;
  }
  else if (request.Resource() == "/blue") {

    bgcolor = "#4444ff";
    title   = "You chose blue";
    body    = "<h1>Blue</h1>" + links;
  }
  else if (request.Resource() == "/form") {

    title   = "Fill a form";

    body    = "<h1>Fill a form</h1>";
    body   += "<form action='/form'>"
              "<table>"
              "<tr><td>Field 1</td><td><input name=field_1></td></tr>"
              "<tr><td>Field 2</td><td><input name=field_2></td></tr>"
              "<tr><td>Field 3</td><td><input name=field_3></td></tr>"
              "</table>"
              "<input type=submit></form>";


    for (std::map<std::string, std::string>::const_iterator i = request.FirstHeader();
         i != request.LastHeader();
         i++)
    {
        body += "<br>" + i->first + " = " + i->second;
    }

    body += "<hr>" + links;
  }
  else {
    response.SetStatus(404, "Not Found");
    title      = "Wrong URL";
    body       = "<h1>Wrong URL</h1>";
    body      += "Path is : &gt;" + request.Resource() + "&lt;"; 
  }

  // response.SetHeader("Connection", "Close");
  response.SetBody("<html><head><title>");
  response.AppendToBody(title);
  response.AppendToBody("</title></head><body bgcolor='" + bgcolor + "'>");
  response.AppendToBody(body);
  response.AppendToBody("</body></html>");

  return false;
}
