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
SHttpRequest::SHttpRequest(SConnection *pConn) :
    scheme("http"),
    host(""),
    port(80),
    method("GET"),
    resource("/"),
    pContentBody(NULL),
    pConnection(pConn),
    pResponse(new SHttpResponse())
{
}

//! Resets to start all over again
void SHttpRequest::Reset(SConnection *pConn)
{
    SHttpMessage::Reset();
    if (pContentBody)
        pContentBody->Reset();

    if (pResponse)
        pResponse->Reset();

    pConnection = pConn;
}

// Clears a http req object
SHttpRequest::~SHttpRequest()
{
    if (pContentBody)
    {
        delete pContentBody;
        pContentBody = NULL;
    }

    if (pResponse)
    {
        delete pResponse;
        pResponse = NULL;
    }
}

// Gets the host
const SString &SHttpRequest::Host() const
{
    return host;
}

// Gets the port
int SHttpRequest::Port() const
{
    return port;
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
    realResource                = r;
    const char *pStart          = r.c_str();
    const char *pEnd            = pStart + r.size();

    // set defaults
    port        = 80;
    queryValues.clear();

    // remove the scheme out - eg http:// or https://
    const char *pColSlashSlash  = strstr(pStart, "://");
    if (pColSlashSlash != NULL)
    {
        scheme = SString(pStart, pColSlashSlash - pStart);
        pStart = pColSlashSlash + 3;
    }
    else
    {
        scheme = "http";
    }

    const char *pHostEnd = pStart;
    // find the end of the host 
    while (*pHostEnd != 0 && *pHostEnd != ':' && *pHostEnd != '/' && *pHostEnd != '?') pHostEnd++;

    if (*pHostEnd == 0)
    {
        host        = pStart;
        resource    = "";
        return ;
    }
    else if (*pHostEnd == ':')
    {
        host = SString(pStart, pHostEnd - pStart);

        // find port and resource
        pStart = ++pHostEnd;
        int newport = 0;
        while (*pHostEnd != 0 && isdigit(*pHostEnd))
        {
            newport = (newport * 10) + (*pHostEnd - '0');
            pHostEnd++;
        }
        if (newport != 0)
            port = newport;
    }

    pStart = pHostEnd;
    while (*pHostEnd != 0 && *pHostEnd != '?') pHostEnd++;
    resource = HttpUtils::Unescape(SString(pStart, pHostEnd - pStart));

    if (*pHostEnd == '?')
    {
        // we have a query
        const char *pNextParam = pHostEnd;
        pStart = pNextParam + 1;

        do
        {
            pNextParam = strchr(pStart, '&');
            if (pNextParam == NULL) pNextParam = pEnd;
            const char *pEqPos = strchr(pStart, '=');
            if (pEqPos != NULL)
            {
                SString qName(pEqPos > pStart ? HttpUtils::Unescape(SString(pStart, pEqPos - pStart)) : "");
                SString qValue(pEqPos + 1 < pNextParam ? HttpUtils::Unescape(SString(pEqPos + 1, pNextParam - (pEqPos + 1))) : "");
                if (qName != "")
                {
                    queryValues.push_back(qName);
                    queryValues.push_back(qValue);
                }
            }
            else
            {
                break ;
            }
            pStart = pNextParam + 1;
        } while (pStart < pEnd);
    }
}

//! Get the value of a query param
SString SHttpRequest::GetQueryValue(const SString &param) const
{
    for (SStringList::const_iterator iter = queryValues.begin(); iter != queryValues.end();++iter)
    {
        if (*iter == param)
        {
            ++iter;
            return *iter;
        }
        else
        {
            ++iter;
        }
    }
    return "";
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

    SetResource(SString(pStart, pCurr - pStart));

    pStart = pCurr;
    while (*pStart && isspace(*pStart)) pStart++;
    if (!pStart) return false;
    version = SString(pStart);

    SLogger::Get()->Log("\nDEBUG: ===============================\n");
    SLogger::Get()->Log("DEBUG: Request: %s %s %s\n",
                         method.c_str(), resource.c_str(), version.c_str());

    return true;
}

// Reads the request line
bool SHttpRequest::ReadFirstLine(std::istream &input)
{
    SString line = HttpUtils::ReadTillCrLf(input);

    return ParseFirstLine(line);

    return true;
}

