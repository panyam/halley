//*****************************************************************************
/*!
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

#include "headers.h"

const std::string TRUE_STRING = "true";
const std::string FALSE_STRING = "false";

//! Write the headers to the stream
bool SHeaderTable::WriteHeaders(std::ostream &output)
{
    // write all headers!
    HeaderMap::const_iterator iter = headers.begin();
    for (;iter != headers.end();++iter)
    {
       output << iter->first << ": " << iter->second << CRLF;
    }
    // and an extra CRLF
    output << CRLF;
    output.flush();

    return true;
}

// Reads a http header.
// Headers are read till a line with only a CRLF is found.
bool SHeaderTable::ReadNextHeader(std::istream &input, std::string &name, std::string &value)
{
    std::string line = ReadTillCrLf(input);

    return ParseHeaderLine(line, name, value);
}

bool SHeaderTable::ReadHeaders(std::istream &input)
{
    std::string headerName;
    std::string headerValue;

    // read all header lines
    while (ReadNextHeader(input, headerName, headerValue)) 
    {
    	std::cerr << "Found Header: " << headerName << ": " << headerValue << std::endl;
    }

    return !input.bad() && !input.eof();
}

//! Parses header line
bool SHeaderTable::ParseHeaderLine(const std::string &line, std::string &name, std::string &value)
{
    const char *pStart  = line.c_str();

    while (isspace(*pStart)) pStart++;
    const char *pCurr = pStart;

    // empty line?
    if (*pCurr == 0) return false;

    while (!iscontrol(*pCurr) && !isseperator(*pCurr))
        pCurr++;

    // found a colon?
    if (*pCurr != ':') return false;

    const char *pHeaderEnd = pCurr;

    // skip the colon
    pCurr++;
    while (isspace(*pCurr)) pCurr++;

    name  = std::string(pStart, pHeaderEnd - pStart);
    value = std::string(pCurr);
    SetHeader(name, value);
    
    return true;
}

// Tells if a header exists
bool SHeaderTable::HasHeader(const std::string &name) const
{
    HeaderMap::const_iterator iter = headers.find(name);
    return iter != headers.end();
}

// Gets a header
std::string SHeaderTable::Header(const std::string &name) const
{
    HeaderMap::const_iterator iter = headers.find(name);
    if (iter == headers.end())
        return "";
    else
        return iter->second;
}

//! Returns a header if it exists
bool SHeaderTable::HeaderIfExists(const std::string &name, std::string &value)
{
    HeaderMap::const_iterator iter = headers.find(name);
    if (iter == headers.end())
        return false;

    value = iter->second;
    return true;
}

// Sets a header value - if it already exists, this value is appended to it.
void SHeaderTable::SetHeader(const std::string &name, const std::string &value, bool append)
{
    if (locked) return ;

    HeaderMap::iterator iter = headers.find(name);
    if (iter != headers.end())
    {
        if (append)
        {
            std::stringstream newvalue;
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

    if (strcasecmp(name.c_str(), "content-length") == 0)
    {
        // contentLength = atoi(value.c_str());
    }
}

//! Sets the value of an bool typed header
void SHeaderTable::SetBoolHeader(const std::string &name, bool value)
{
    SetHeader(name, value ? TRUE_STRING : FALSE_STRING);
}

//! Sets the value of an int typed header
void SHeaderTable::SetUIntHeader(const std::string &name, unsigned value)
{
    char valueStr[64];
    sprintf(&valueStr[0], "%u", value);
    SetHeader(name, &valueStr[0]);
}

//! Sets the value of an int typed header
void SHeaderTable::SetIntHeader(const std::string &name, int value)
{
    char valueStr[64];
    sprintf(valueStr, "%d", value);
    SetHeader(name, &valueStr[0]);
}

//! Sets the value of an double typed header
void SHeaderTable::SetDoubleHeader(const std::string &name, double value)
{
    char valueStr[64];
    sprintf(valueStr, "%f", value);
    SetHeader(name, &valueStr[0]);
}

// Removes a header
std::string SHeaderTable::RemoveHeader(const std::string &name)
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

