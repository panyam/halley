//*****************************************************************************
/*!
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

