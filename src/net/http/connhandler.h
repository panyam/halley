//*****************************************************************************
/*!
 *  \file   httpclient.h
 *
 *  \brief
 *  A simple http client handler.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_HANDLER_H_
#define _SHTTP_HANDLER_H_

#include "net/connhandler.h"
#include <map>

class SHttpRequest;
class SHttpResponse;
class SHttpMessage;
class SHttpMPMessage;

typedef enum
{
    INPUT_HANDLER,
    REQUEST_HANDLER,
    OUTPUT_HANDLER
} SHandlerType;

//! A handler in the handler chain.
//
// TODO:
// The ProcessXXX functions return true or false to indicate whether the
// next chain is to be handled or not
//
// We may perhaps need another output to indicate that all processing
// should stop and the response that is now created must be returned as is.
// How should we handle this?
//
class SHttpRequestHandler
{
public:
    //! Empty virtual destructor to safeguard against some compilers
    virtual ~SHttpRequestHandler() { }

    //! Handles a request - in the input phase.
    //
    // Returns:
    //  true if processing should stop
    //  false if processing should continue to the next handler
    virtual bool ProcessRequest(SHttpRequest &req, SHttpResponse &resp) { return false; }

    //! Handles a response - in the output phase
    //
    // Returns:
    //  true if processing should stop
    //  false if processing should continue to the next handler
    virtual bool ProcessResponse(SHttpRequest &req, SHttpResponse &resp) { return false; }

    //! This processes each multi part message as it is sent out
    virtual bool ProcessMessage(SHttpRequest &   req,
                                SHttpResponse &  resp,
                                SHttpMessage &   msg) { return false; }
};


//*****************************************************************************
/*!
 *  \class  SHttpConnHandler
 *
 *  \brief  A http connection handler.
 *
 *****************************************************************************/
class SHttpConnHandler : public SConnHandler
{
public:
    //! Creates a handler
    SHttpConnHandler();

    //! Destroys the handler
    virtual         ~SHttpConnHandler();

    //! Registers a request handler
    virtual void    RegisterRequestHandler(SHttpRequestHandler *pHandler, SHandlerType handlerType, int index = -1);

    //! Unregisters a request handler
    virtual SHttpRequestHandler *UnRegisterRequestHandler(SHandlerType handlerType, int index = 0);

public:
    //! Tries to open a file and returns a stringified error if open failed.
    static FILE *   OpenFile(const char *filename, const char *mode, std::string &errormsg);

    // Get the file type
    static std::string     GetFileType(const std::string &filename);

protected:
    // Read a request from the socket
    int             ReadRequest(SHttpRequest &request);

    // Handle a new connection
    virtual bool    HandleConnection() { Start(); return true; }

    virtual int     Run();

protected:
    typedef std::list<SHttpRequestHandler *>   HandlerList;

    //! The input, request and output handlers
    HandlerList     handlers[3];
};

#endif

