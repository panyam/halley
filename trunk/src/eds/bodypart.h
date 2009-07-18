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

#ifndef _SBODY_PART_H_
#define _SBODY_PART_H_

#include "fwd.h"

//*****************************************************************************
/*!
 *  \class  SBodyPart
 *
 *  \brief  A single chunk of content data that can be generated, modified or
 *  transmitted by a module.
 *
 *****************************************************************************/
class SBodyPart
{
public:
    enum
    {
        BP_RAW,         // body data contains the raw data to be transmitted
        BP_FILE,        // body data contains the name of the file to send/transmit
        BP_LAZY,        // Lazy body types.  Often, we dont want to create body 
                        // parts between modules, we want the ability of a later 
                        // stage module to call an earlier stage module on demand 
                        // when it needs it and also with "howmuch" it needs given 
                        // the size of its buffers.
        BP_NUM_TYPES    // define other types from here
    };

public:
    // Creates the body part
    SBodyPart(int btype = BP_RAW, unsigned index = 0, void *data = NULL);

    // Token Destructor
    virtual ~SBodyPart() { }

    //! Writes the body to stream from a given offset
    virtual int WriteToStream(std::ostream &output, int from = 0) = 0;

    //! Writes the body to the connection from a given offset
    virtual bool WriteToConnection(SConnection *pConn, int &numWritten, int from = 0) = 0;

    //! Clears the body part for later use!
    virtual void Reset() { }

    //! Gets the index of the bpart
    inline BPIndexType Index() { return bpIndex; }

    //! Body part type
    inline int Type() { return bpType; }

    //! Return the extra data.
    template <typename T> T ExtraData() { return reinterpret_cast<T>(extra_data); }

public:
    // Type of body part
    int         bpType;

    // Index of the body part - for ordering of body part handling
    BPIndexType bpIndex;

    //! The handler data
    void *      extra_data;
};

// A comparison functor for comparing two body parts based on the order
// of transmission.
class SBodyPartComparer
{
public:
    bool operator()(const SBodyPart *a, const SBodyPart *b) const;
};

typedef std::vector<SBodyPart *>    SBodyPartVector;
typedef std::list<SBodyPart *>      SBodyPartList;
// A priority queue of body parts
typedef std::priority_queue<SBodyPart *, SBodyPartVector, SBodyPartComparer>    SBodyPartQueue;

//*****************************************************************************
/*!
 *  \class  SRawBodyPart
 *
 *  \brief  Body parts that contain the raw content/data.
 *
 *****************************************************************************/
class SRawBodyPart : public SBodyPart
{
public:
    // Creates the body part
    SRawBodyPart(unsigned index = 0, void *data = NULL)
        : SBodyPart(BP_RAW, index, data) { }

    // Token Destructor
    virtual ~SRawBodyPart() { data.clear(); }

    //! Sets a string as the body
    void SetBody(const SString &data);

    //! Sets raw bytes as the body
    void SetBody(const char *buffer, unsigned size);

    //! Inserts content at a given position
    void InsertInBody(const SString &data, size_t offset = 0);

    //! Insert raw bytes into the body
    void InsertInBody(const char *buffer, unsigned size, size_t offset = 0);

    //! Appends a string to the body
    void AppendToBody(const SString &data);

    //! Appends raw bytes to the body
    void AppendToBody(const char *buffer, unsigned size);

    //! Gets the body
    const SCharVector &Body() const;

    //! Get the data size
    inline int Size() { return data.size(); }

    //! Writes the body to stream - override for multipart messages
    virtual int WriteToStream(std::ostream &output, int from = 0);

    //! Writes the body to an FD - override for multipart messages
    virtual bool WriteToConnection(SConnection *pConn, int &numWritten, int from = 0);

public:
    //! The data required for this body part
    SCharVector data;
};

//*****************************************************************************
/*!
 *  \class  SFileBodyPart
 *
 *  \brief  Body parts that contains info about the file to be transmitted
 *  instead of their contents.
 *
 *****************************************************************************/
class SFileBodyPart : public SBodyPart
{
public:
    SFileBodyPart(const SString &fname, unsigned index = 0, void *data = NULL)
        : SBodyPart(BP_FILE, index, data), filename(fname) { }

    virtual ~SFileBodyPart() { }

    //! Writes the body to stream from a given offset
    virtual int WriteToStream(std::ostream &output, int from = 0);

    //! Writes the body to an FD from a given offset
    virtual bool WriteToConnection(SConnection *pConn, int &numWritten, int from = 0);

public:
    SString filename;
};

//*****************************************************************************
/*!
 *  \class  SLazyBodyPart
 *
 *  \brief  A lazy body part - uses callbacks to generate data on fly
 *  (perhaps even more than once).
 *
 *****************************************************************************/
class SLazyBodyPart : public SBodyPart
{
public:
    SLazyBodyPart(unsigned index = 0, void *data = NULL)
        : SBodyPart(BP_LAZY, index, data) { }

    virtual ~SLazyBodyPart() { }

    //! Writes the body to stream from a given offset
    virtual int WriteToStream(std::ostream &output, int from = 0);

    //! Writes the body to an FD from a given offset
    virtual bool WriteToConnection(SConnection *pConn, int &numWritten, int from = 0);
};

#endif

