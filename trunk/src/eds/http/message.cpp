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

// Creates a new http message object
SHttpMessage::SHttpMessage()
{
    version         = "HTTP/1.1";
    bpCount         = 0;
    headers.SetHeader("Content-Type", "text/html");
}

// Clears a http req object
SHttpMessage::~SHttpMessage()
{
}

//! Resets to start all over again
void SHttpMessage::Reset()
{
    headers.Reset();
    version         = "HTTP/1.1";
    bpCount         = 0;
    headers.SetHeader("Content-Type", "text/html");
}

// Creates a new body part for this message
SRawBodyPart *SHttpMessage::NewRawBodyPart(void *extra_data)
{
    return new SRawBodyPart(bpCount++, extra_data);
}

// Creates a new body part for this message
SFileBodyPart *SHttpMessage::NewFileBodyPart(const SString &filename, void *extra_data)
{
    return new SFileBodyPart(filename, bpCount++, extra_data);
}

//! Returns a part that indicates end of content
SRawBodyPart *SHttpMessage::NewContFinishedPart(SHttpModule *pNextModule)
{
    SRawBodyPart *pPart = NewRawBodyPart(pNextModule);
    pPart->bpType = SHttpMessage::HTTP_BP_CONTENT_FINISHED;
    return pPart;
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
const SString &SHttpMessage::Version() const
{
    return version;
}

//! Gets the content length
int SHttpMessage::ContentLength()
{
    SString length;
    if (headers.HeaderIfExists("Content-Length", length))
        return atoi(length.c_str());
    return 0;
}

//! Tells if a response is multipart or not
bool SHttpMessage::IsMultipart()
{
    SString hdrContType;
    headers.HeaderIfExists("Content-Type", hdrContType);
    return (strncmp("multipart", hdrContType.c_str(), 9) == 0);
}

// Sets the version
void SHttpMessage::SetVersion(const SString &v)
{
    version = v;
}

//! Write the message to a stream
int SHttpMessage::WriteToStream(std::ostream &output)
{
    return headers.WriteToStream(output);
}


//! Write the message to a FD
int SHttpMessage::WriteToFD(int fd)
{
    return headers.WriteToFD(fd);
}



