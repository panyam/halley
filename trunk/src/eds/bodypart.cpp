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

#include "utils.h"
#include "bodypart.h"
#include "connection.h"

// Creates a new body part
SBodyPart::SBodyPart(int bType, unsigned index, void *d)
    : bpType(bType), bpIndex(index), extra_data(d)
{
}

//! Compares 2 body parts based on their indices
bool SBodyPartComparer::operator()(const SBodyPart *a, const SBodyPart *b) const
{
    return b->bpIndex < a->bpIndex;
}

/************************************************************************
 *
 *                      Raw Body Parts
 *
 ***********************************************************************/

// Gets the body data.
const SCharVector &SRawBodyPart::Body() const
{
    return data;
}

// Sets the content
void SRawBodyPart::SetBody(const SString &data)
{
    SetBody(data.c_str(), data.size());
}

// Sets the content
void SRawBodyPart::SetBody(const char *buffer, unsigned size)
{
    data.assign(buffer, buffer + size);
}

// Inserts into the body
void SRawBodyPart::InsertInBody(const SString &data, size_t offset)
{
    InsertInBody(data.c_str(), data.size());
}

// Inserts into the body
void SRawBodyPart::InsertInBody(const char *buffer, unsigned size, size_t offset)
{
    data.insert(data.begin() + offset, buffer, buffer + size);
}

// Appends to the body
void SRawBodyPart::AppendToBody(const SString &data)
{
    AppendToBody(data.c_str(), data.size());
}

// Appends to the body
void SRawBodyPart::AppendToBody(const char *buffer, unsigned size)
{
    data.insert(data.end(), buffer, buffer + size);
}

//! Writes body part to a stream
int SRawBodyPart::WriteToStream(std::ostream &output, int from)
{
    copy(data.begin() + from, data.end(), std::ostreambuf_iterator<char>(output));
    output.flush();
    return data.size() - from;
}

//! Writes body part to a FD
bool SRawBodyPart::WriteToConnection(SConnection *pConn, int &numWritten)
{
    int length = data.size() - bytesWritten;
    numWritten = pConn->WriteData(&data[bytesWritten], length);
    if (numWritten > 0)
        bytesWritten += numWritten;
    return numWritten != length;
}

/************************************************************************
 *
 *                              File Body Parts
 *
 ***********************************************************************/
//! Creates a new file body part
SFileBodyPart::SFileBodyPart(const SString &fname, size_t fsize,
                             unsigned index, void *data)
:
    SBodyPart(BP_FILE, index, data),
    filename(fname),
    readFD(-1),
    filesize(fsize),
    offset(0)
{
}

//! Writes body part to a stream
int SFileBodyPart::WriteToStream(std::ostream &output, int from)
{
    return 0;
}

//! Writes body part to a FD
bool SFileBodyPart::WriteToConnection(SConnection *pConn, int &numWritten)
{
    if (readFD < 0)
    {
        // then open it
        if ((readFD = open(filename.c_str(), O_RDONLY)) < 0)
        {
            // error
            return readFD;
        }
    }

    int length = filesize - offset;
    numWritten = sendfile(pConn->Socket(), readFD, &offset, length);

    if (numWritten == length)
    {
        // close the file if we are done with it
        close(readFD);
        readFD = -1;
    }
    return numWritten != length;
}

/************************************************************************
 *
 *                              Lazy Body Parts
 *
 ***********************************************************************/
//! Writes body part to a stream
int SLazyBodyPart::WriteToStream(std::ostream &output, int from)
{
    return 0;
}

//! Writes body part to a FD
bool SLazyBodyPart::WriteToConnection(SConnection *pConn, int &numWritten)
{
    return false;
}

