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
 *  \file   fileiohelper.cpp
 *
 *  \brief  The stage that does asynchronous file io.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "fileiohelper.h"

// number of file handles to keep track 
// of at any given time
#define MAXEPOLLSIZE 100

// Creates a new file io helper stage
SFileIOHelper::SFileIOHelper()
{
}

//! Open a file handle
void SFileIOHelper::OpenFile(const char *       path,
                             int                flags,
                             SFileIOListener *  pListener)
{
    pListener->FileOpened(open(path, flags));
}

//! Read bytes
void SFileIOHelper::ReadBytes(int               fd,
                              size_t            nbytes,
                              char *            buffer,
                              SFileIOListener * pListener)
{
    pListener->DataRead(fd, read(fd, buffer, nbytes), buffer);
}

//! Write bytes
void SFileIOHelper::WriteBytes(int              fd,
                               size_t           nbytes,
                               const char *     buffer,
                               SFileIOListener *pListener)
{
    pListener->DataWritten(fd, write(fd, buffer, nbytes));
}

