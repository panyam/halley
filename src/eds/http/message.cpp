//*****************************************************************************
/*!
 *  \file   message.cpp
 *
 *  \brief
 *  A message object.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "message.h"

// Creates a new body part
SBodyPart::SBodyPart(unsigned index, int bType, void *d)
    : bpIndex(index), bpType(bType), extra_data(d)
{
}

//! Writes body part to a stream
bool SBodyPart::WriteMessageBody(std::ostream &output)
{
    copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(output));
    output.flush();
    return true;
}

// Gets the body data.
const std::vector<char> &SBodyPart::Body() const
{
    return data;
}

// Resets the body part
void SBodyPart::Reset()
{
    data.clear();
}

// Sets the content
void SBodyPart::SetBody(const std::string &data)
{
    SetBody(data.c_str(), data.size());
}

// Sets the content
void SBodyPart::SetBody(const char *buffer, unsigned size)
{
    data.assign(buffer, buffer + size);
}

// Prepends to the body
void SBodyPart::PrependToBody(const std::string &data)
{
    PrependToBody(data.c_str(), data.size());
}

// Prepends to the body
void SBodyPart::PrependToBody(const char *buffer, unsigned size)
{
    data.insert(data.begin(), buffer, buffer + size);
}

// Appends to the body
void SBodyPart::AppendToBody(const std::string &data)
{
    AppendToBody(data.c_str(), data.size());
}

// Appends to the body
void SBodyPart::AppendToBody(const char *buffer, unsigned size)
{
    data.insert(data.end(), buffer, buffer + size);
}

// Creates a new http message object
SHttpMessage::SHttpMessage()
{
    version         = "HTTP/1.1";
    bpCount         = 0;
    // SetUIntHeader("Content-Length", 0);
    headers.SetHeader("Content-Type", "text/html");
}

// Clears a http req object
SHttpMessage::~SHttpMessage()
{
}

// Creates a new body part for this message
SBodyPart *SHttpMessage::NewBodyPart(int bptype, void *extra_data)
{
    return new SBodyPart(bpCount++, bptype, extra_data);
}

// Reads the message body
bool SHttpMessage::ReadMessageBody(std::istream &input)
{
    return false;
}

// Read the message from an input stream
bool SHttpMessage::ReadFromStream(std::istream &input)
{
    return ReadFirstLine(input) && headers.ReadHeaders(input) && ReadMessageBody(input);
}

// Gets the version
const std::string &SHttpMessage::Version() const
{
    return version;
}

//! Gets the content length
int SHttpMessage::ContentLength()
{
    std::string length;
    if (headers.HeaderIfExists("Content-Length", length))
        return atoi(length.c_str());
    return 0;
}

//! Tells if a response is multipart or not
bool SHttpMessage::IsMultipart()
{
    std::string hdrContType;
    headers.HeaderIfExists("Content-Type", hdrContType);
    return (strncmp("multipart", hdrContType.c_str(), 9) == 0);
}

// Sets the version
void SHttpMessage::SetVersion(const std::string &v)
{
    version = v;
}

//! Write the message to a stream
bool SHttpMessage::WriteToStream(std::ostream &output)
{
    headers.WriteHeaders(output);

    /*
    for (std::list<SBodyPart *>::iterator iter = bodyParts.begin(); iter != bodyParts.end();++iter)
    {
        if (!(*iter)->WriteMessageBody(output))
        {
            return false;
        }
    }
    */
    return true;
}

