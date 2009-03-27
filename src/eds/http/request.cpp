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

#include "request.h"
#include "response.h"

#include <sstream>
#include <string.h>

// Creates a new http request object
SHttpRequest::SHttpRequest() :
    method("GET"),
    resource("/"),
    pContentBody(NULL),
    pResponse(new SHttpResponse())
{
}

// Clears a http req object
SHttpRequest::~SHttpRequest()
{
    if (pContentBody)
        delete pContentBody;

    if (pResponse)
        delete pResponse;
}

// Gets the method
const SString &SHttpRequest::Method() const
{
    return method;
}

// Sets the method
void SHttpRequest::SetMethod(const SString &m)
{
    method = m;
}

// Gets the method Resource
const SString &SHttpRequest::Resource() const
{
    return resource;
}

// Sets the request Resource
void SHttpRequest::SetResource(const SString &r)
{
    resource = r;
}

//! Parses the first line
bool SHttpRequest::ParseFirstLine(const SString &line)
{
    const char *pStart  = line.c_str();

    while (*pStart && isspace(*pStart)) pStart++;
    const char *pCurr = pStart;
    while (*pCurr && !isspace(*pCurr)) pCurr++;
    
    method = SString(pStart, pCurr - pStart);

    pStart = pCurr;
    while (*pStart && isspace(*pStart)) pStart++;

    if (!pStart) return false;
    pCurr = pStart;
    while (*pCurr && !isspace(*pCurr)) pCurr++;

    resource = SString(pStart, pCurr - pStart);

    // TODO: Unescape the resource
    // TODO: Strip ./ and ../ from resource

    pStart = pCurr;
    while (*pStart && isspace(*pStart)) pStart++;
    if (!pStart) return false;
    version = SString(pStart);

    return true;
}

// Reads the request line
bool SHttpRequest::ReadFirstLine(std::istream &input)
{
    SString line = ReadTillCrLf(input);

    return ParseFirstLine(line);

    return true;
}

