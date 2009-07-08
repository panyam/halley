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

//! Resets to start all over again
void SHttpResponse::Reset()
{
    SHttpMessage::Reset();
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
    output << version << " " << statusCode << " " << statusMessage << HttpUtils::CRLF;

    // set the content length
    // SetUIntHeader("Content-Length", body.size());

    return SHttpMessage::WriteToStream(output);
}

