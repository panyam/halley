//*****************************************************************************
/*!
 *  \file   request.cpp
 *
 *  \brief
 *  A request object.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "response.h"
#include "json/json.h"

#include <sstream>
#include <iterator>

// Creates a new http request object
SHttpResponse::SHttpResponse()
{
    statusCode      = 200;
    statusMessage   = "OK";
}

// Clears a http req object
SHttpResponse::~SHttpResponse()
{
}

// Sets the request status
int SHttpResponse::StatusCode() const
{
    return statusCode;
}

// The status message
const SString SHttpResponse::StatusMessage() const
{
    return statusMessage;
}

// Sets the request status
void SHttpResponse::SetStatus(int status, const SString &msg)
{
    statusCode = status;
    statusMessage = msg;
}

//! Writes the response to a stream
bool SHttpResponse::WriteToStream(std::ostream &output)
{
    output << version << " " << statusCode << " " << statusMessage << CRLF;

    // set the content length
    // SetUIntHeader("Content-Length", body.size());

    return SHttpMessage::WriteToStream(output);
}

