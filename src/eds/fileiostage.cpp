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
 *  \file   fileiostage.cpp
 *
 *  \brief  The stage that does asynchronous file io.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#include "fileiostage.h"

// Creates a new file io helper stage
SFileIOStage::SFileIOStage(int numThreads) : SStage(numThreads)
{
}

// Open a file handle
void SFileIOStage::OpenFile(void *pSource, const char *path, const char *mode)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->fileName        = strdup(path);
    strcpy(pEvent->fileMode, mode);
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_OPEN, pSource, pEvent));
}

// Read bytes
void SFileIOStage::ReadBytes(void *pSource, FILE *file, size_t nbytes)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->filePtr     = file;
    pEvent->numBytes    = nbytes;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_READ, pSource, pEvent));
}

// Write bytes
void SFileIOStage::WriteBytes(void *pSource, FILE *file, size_t nbytes)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->filePtr     = file;
    pEvent->numBytes    = nbytes;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_WRITE, pSource, pEvent));
}

//! Change the current offset of a file
void SFileIOStage::SeekOffset(void *pSource, FILE *file, long offset)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->offset      = offset;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_SEEKOFFSET, pSource, pEvent));
}

