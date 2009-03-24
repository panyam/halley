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
 *  \file   message.h
 *
 *  \brief
 *  A HTTP Message.
 *
 *  \version
 *      - S Panyam      13/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_MESSAGE_H_
#define _SHTTP_MESSAGE_H_

#include <list>
#include <vector>
#include <algorithm>

#include "headers.h"

class SBodyPart
{
public:
    enum
    {
        BP_NORMAL,  // a normal body is being sent
        BP_OPEN_SUB_MESSAGE,    // a command to open a sub message - boundary will in body part
        BP_CLOSE_SUB_MESSAGE,   // close the last sub message
        BP_CLOSE_CONNECTION,    // close the connection
        BP_CONTENT_FINISHED,    // Denotes that content is finished (but dont have to close connection)

        BP_NUM_TYPES            // define new types after this
    };

public:
    // Creates the body part
    SBodyPart(unsigned index = 0, int btype = BP_NORMAL, void *data = NULL);

    // Token Destructor
    virtual ~SBodyPart() { }

    //! Clears the body part for later use!
    void Reset();

    //! Prepends a string to the body
    void PrependToBody(const std::string &data);

    //! Prepends raw bytes to the body
    void PrependToBody(const char *buffer, unsigned size);

    //! Sets a string as the body
    void SetBody(const std::string &data);

    //! Sets raw bytes as the body
    void SetBody(const char *buffer, unsigned size);

    //! Appends a string to the body
    void AppendToBody(const std::string &data);

    //! Appends raw bytes to the body
    void AppendToBody(const char *buffer, unsigned size);

    //! Gets the body
    const std::vector<char> &Body() const;

    //! Writes the body to stream - override for multipart messages
    virtual bool WriteMessageBody(std::ostream &output);

    //! Get the data size
    inline int Size() { return data.size(); }

    //! Gets the index of the bpart
    inline BPIndexType Index() { return bpIndex; }

    //! Body part type
    inline int Type() { return bpType; }

    //! Return the extra data.
    template <typename T> T ExtraData() { return reinterpret_cast<T>(extra_data); }

public:
    // Index of the body part - for ordering of body part handling
    BPIndexType bpIndex;

    // Type of body part
    int         bpType;

    //! The data required for this body part
    std::vector<char>   data;

    //! The handler data
    void * extra_data;
};

//*****************************************************************************
/*!
 *  \class  SHttpMessage
 *
 *  \brief  A http message object.
 *
 *****************************************************************************/
class SHttpMessage
{
public:
    SHttpMessage();
    virtual         ~SHttpMessage();

public:
    //! Read request from an input stream (usually network)
    virtual bool ReadFromStream(std::istream &input);

    //! Write the response to a stream
    virtual bool WriteToStream(std::ostream &output);

    //! Gets the version
    const std::string &Version() const;

    //! Gets the content length
    virtual int ContentLength();

    //! Tells if the message is multipart
    virtual bool IsMultipart();

    //! Gets the headers
    SHeaderTable &Headers() { return headers; }

    //! Sets the version
    void SetVersion(const std::string &version);

    //! Creates a new body part
    SBodyPart *NewBodyPart(int bptype = SBodyPart::BP_NORMAL, void *extra_data = NULL);

public:
    //! Reads the next header
    virtual bool ReadFirstLine(std::istream &input) { return false; }

    //! Reads the message body
    virtual bool ReadMessageBody(std::istream &input);

protected:
    //! Version - eg HTTP/1.1
    std::string     version;

    //! The header table
    SHeaderTable    headers;

    // Number of body parts in this message so far
    // TODO - how to handle more than 4 billion messages?
    BPIndexType     bpCount;
};

#endif

