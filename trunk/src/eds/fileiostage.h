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
 *  \file   fileiostage.h
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

#include "stage.h"

//! Events for file IO
class SFileIOEvent
{
public:
    enum
    {
        EVT_FILE_OPEN,
        EVT_FILE_READ,
        EVT_FILE_WRITE,
        EVT_FILE_SEEKOFFSET,
        EVT_FILE_CLOSE
    };

public:
    SFileIOEvent()
    {
        filePtr     = NULL;
        fileName    = NULL;
        fileMode[0] = 0;
        numBytes    = 0;
        offset      = 0;
    }

    virtual ~SFileIOEvent()
    {
        if (filePtr != NULL) fclose(filePtr);
        if (fileName != NULL) free(fileName);
    }

    //! File PTR
    FILE *  filePtr;

    //! Name of the file to read
    char *  fileName;

    //! file mode
    char    fileMode[16];

    //! Size of the read/write 
    size_t  numBytes;

    //! Offset of the access
    long    offset;
};

//*****************************************************************************
/*!
 *  \class  SFileIOStage
 *
 *  \brief  The stage that does asynchronous file IO by hiding away all the
 *  different implementation issues regarding blocking file IO.
 *
 *****************************************************************************/
class SFileIOStage : public SStage
{
public:
    // Creates a new fileio helper stage
    SFileIOStage(int numThreads = 0);
    
    // Destroys the stage
    virtual ~SFileIOStage() { }

    // Open a file handle
    void    OpenFile(void *pSource, const char *path, const char *mode);

    // Read bytes
    void    ReadBytes(void *pSource, FILE *file, size_t nbytes);

    // Write bytes
    void    WriteBytes(void *pSource, FILE *file, size_t nbytes);

    //! Change the current offset of a file
    void    SeekOffset(void *pSource, FILE *file, long offset);
};

#endif

