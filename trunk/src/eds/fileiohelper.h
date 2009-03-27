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
 *  \file   fileiohelper.h
 *
 *  \brief  The stage that does asynchronous file io.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SFILEIO_STAGE_H_
#define _SFILEIO_STAGE_H_

#include "thread/task.h"

//! Callback for file io helper
class SFileIOListener
{
public:
    //! Destroyes the listener
    virtual ~SFileIOListener() { }

    //! Called when a file is opened
    virtual void FileOpened(int fd) { }

    //! Called when data is read
    virtual void DataRead(int fd, int numRead, char *buffer) { }

    //! Called when data is written
    virtual void DataWritten(int fd, int numWritten) { }
};

//*****************************************************************************
/*!
 *  \class  SFileIOHelper
 *
 *  \brief  The stage that does asynchronous file IO by hiding away all the
 *  different implementation issues regarding blocking file IO.
 *
 *****************************************************************************/
class SFileIOHelper
{
public:
    // Creates a new fileio helper stage
    SFileIOHelper();
    
    // Destroys the stage
    virtual ~SFileIOHelper() { }

    // Open a file handle
    void    OpenFile(const char *       path,
                     int                flags,
                     SFileIOListener *  pListener);

    // Read bytes
    void    ReadBytes(int               fd,
                      size_t            nbytes,
                      char *            buffer,
                      SFileIOListener * pListener);

    // Write bytes
    void    WriteBytes(int              fd,
                       size_t           nbytes,
                       const char *     buffer,
                       SFileIOListener *pListener);
};

#endif

