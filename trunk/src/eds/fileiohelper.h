//*****************************************************************************
/*!
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

