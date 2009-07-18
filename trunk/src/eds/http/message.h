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

#include "headers.h"
#include "../bodypart.h"
 
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
    /**
     * HTTP specific Body Part types.
     */
    enum
    {
        HTTP_BP_START = SBodyPart::BP_NUM_TYPES,
        HTTP_BP_OPEN_SUB_MESSAGE,   // a command to open a sub message - 
                                    // boundary will in body part
        HTTP_BP_CLOSE_SUB_MESSAGE,  // close the last sub message
        HTTP_BP_CLOSE_CONNECTION,   // close the connection
        HTTP_BP_CONTENT_FINISHED,   // Denotes that content is finished 
                                    // (but dont have to close connection)
        HTTP_BP_NUM_TYPES           // Where HTTP specific body part ends
    };
public:
    SHttpMessage();
    virtual         ~SHttpMessage();

    //! Resets to start all over again
    virtual void Reset();

public:
    //! Read request from an input stream (usually network)
    virtual bool ReadFromStream(std::istream &input);

    //! Write the message to a stream
    virtual int WriteToStream(std::ostream &output);

    //! Write the message to a FD
    virtual int WriteToFD(int fd);

    //! Gets the version
    const SString &Version() const;

    //! Gets the content length
    virtual int ContentLength();

    //! Tells if the message is multipart
    virtual bool IsMultipart();

    //! Gets the headers
    SHeaderTable &Headers() { return headers; }

    //! Sets the version
    void SetVersion(const SString &version);

    //! Creates a new body part
    SBodyPart *NewBodyPart(int bptype = SBodyPart::BP_NORMAL, void *extra_data = NULL);

public:
    //! Reads the next header
    virtual bool ReadFirstLine(std::istream &input) { return false; }

    //! Reads the message body
    virtual bool ReadMessageBody(std::istream &input);

protected:
    //! Version - eg HTTP/1.1
    SString     version;

    //! The header table
    SHeaderTable    headers;

    // Number of body parts in this message so far
    BPIndexType     bpCount;
};

#endif

