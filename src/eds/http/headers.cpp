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
 *  \file   headers.cpp
 *
 *  \brief
 *  All things http headers.
 *
 *  \version
 *      - Sri Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include "../utils.h"
#include "headers.h"

const SString TRUE_STRING = "true";
const SString FALSE_STRING = "false";

//! CLears the header table so we can start over again
void SHeaderTable::Reset()
{
    closeConnection = false;
    locked          = false;
    headers.clear();
}

//! Write the headers to the stream
int SHeaderTable::WriteToStream(std::ostream &output)
{
    // write all headers!
    HeaderMap::const_iterator iter = headers.begin();
    for (;iter != headers.end();++iter)
    {
        output << iter->first << ": " << iter->second << URLUtils::CRLF;
        SLogger::Get()->Log("DEBUG: Header      %s: %s\n", iter->first.c_str(), iter->second.c_str());
    }
    // and an extra URLUtils::CRLF
    output << URLUtils::CRLF;
    output.flush();

    return 0;
}

//! Writes the headers to a file descriptor
int SHeaderTable::WriteToFD(int fd)
{
    SStringStream buffer;
    WriteToStream(buffer);
    SString outStr(buffer.str());
    return SEDSUtils::SendFully(fd, outStr.c_str(), outStr.size());
}

// Reads a http header.
// Headers are read till a line with only a CRLF is found.
bool SHeaderTable::ReadNextHeader(std::istream &input, SString &name, SString &value)
{
    SString line = URLUtils::ReadTillCrLf(input);

    return ParseHeaderLine(line, name, value);
}

bool SHeaderTable::ReadHeaders(std::istream &input)
{
    SString headerName;
    SString headerValue;

    // read all header lines
    while (ReadNextHeader(input, headerName, headerValue)) { }

    return !input.bad() && !input.eof();
}

//! Parses header line
bool SHeaderTable::ParseHeaderLine(const SString &line, SString &name, SString &value)
{
    const char *pStart  = line.c_str();

    while (isspace(*pStart)) pStart++;
    const char *pCurr = pStart;

    // empty line?
    if (*pCurr == 0) return false;

    while (!URLUtils::iscontrol(*pCurr) && !URLUtils::isseperator(*pCurr))
        pCurr++;

    // found a colon?
    if (*pCurr != ':') return false;

    const char *pHeaderEnd = pCurr;

    // skip the colon
    pCurr++;
    while (isspace(*pCurr)) pCurr++;

    name  = SString(pStart, pHeaderEnd - pStart);
    value = SString(pCurr);
    SLogger::Get()->Log("DEBUG: Header      %s: %s\n", name.c_str(), value.c_str());
    SetHeader(name, value);
    
    return true;
}

// Tells if a header exists
bool SHeaderTable::HasHeader(const SString &name) const
{
    HeaderMap::const_iterator iter = headers.find(name);
    return iter != headers.end();
}

// Gets a header
SString SHeaderTable::Header(const SString &name) const
{
    HeaderMap::const_iterator iter = headers.find(name);
    if (iter == headers.end())
        return "";
    else
        return iter->second;
}

//! Returns a header if it exists
bool SHeaderTable::HeaderIfExists(const SString &name, SString &value)
{
    HeaderMap::const_iterator iter = headers.find(name);
    if (iter == headers.end())
        return false;

    value = iter->second;
    return true;
}

// Sets a header value - if it already exists, this value is appended to it.
void SHeaderTable::SetHeader(const SString &name, const SString &value, bool append)
{
    if (locked) return ;

    HeaderMap::iterator iter = headers.find(name);
    if (iter != headers.end())
    {
        if (append)
        {
            SStringStream newvalue;
            newvalue << iter->second << "," << value;
            headers.insert(HeaderPair(name, newvalue.str()));
        }
        else
        {
            headers[name] = value;
        }
    }
    else
    {
        headers.insert(HeaderPair(name, value));
    }

    if ((strcasecmp(name.c_str(), "Connection") == 0) &&
        (strcasecmp(value.c_str(), "close") == 0))
    {
        closeConnection = true;
    }

    if (strcasecmp(name.c_str(), "content-length") == 0)
    {
        // contentLength = atoi(value.c_str());
    }
}

//! Sets the value of an bool typed header
void SHeaderTable::SetBoolHeader(const SString &name, bool value)
{
    SetHeader(name, value ? TRUE_STRING : FALSE_STRING);
}

//! Sets the value of an int typed header
void SHeaderTable::SetUIntHeader(const SString &name, unsigned value)
{
    char valueStr[64];
    sprintf(&valueStr[0], "%u", value);
    SetHeader(name, &valueStr[0]);
}

//! Sets the value of an int typed header
void SHeaderTable::SetIntHeader(const SString &name, int value)
{
    char valueStr[64];
    sprintf(valueStr, "%d", value);
    SetHeader(name, &valueStr[0]);
}

//! Sets the value of an double typed header
void SHeaderTable::SetDoubleHeader(const SString &name, double value)
{
    char valueStr[64];
    sprintf(valueStr, "%f", value);
    SetHeader(name, &valueStr[0]);
}

// Removes a header
SString SHeaderTable::RemoveHeader(const SString &name)
{
    if ( ! locked)
    {
        HeaderMap::iterator iter = headers.find(name);
        if (iter != headers.end())
        {
            headers.erase(iter);
            return iter->second;
        }
    }
    return "";
}

