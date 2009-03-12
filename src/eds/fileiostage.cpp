//*****************************************************************************
/*!
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
void SFileIOStage::OpenFile(SJob *pJob, const char *path, const char *mode)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->fileName        = strdup(path);
    strcpy(pEvent->fileMode, mode);
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_OPEN, pJob, pEvent));
}

// Read bytes
void SFileIOStage::ReadBytes(SJob *pJob, FILE *file, size_t nbytes)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->filePtr     = file;
    pEvent->numBytes    = nbytes;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_READ, pJob, pEvent));
}

// Write bytes
void SFileIOStage::WriteBytes(SJob *pJob, FILE *file, size_t nbytes)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->filePtr     = file;
    pEvent->numBytes    = nbytes;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_WRITE, pJob, pEvent));
}

//! Change the current offset of a file
void SFileIOStage::SeekOffset(SJob *pJob, FILE *file, long offset)
{
    SFileIOEvent *pEvent    = new SFileIOEvent();
    pEvent->offset      = offset;
    QueueEvent(SEvent(SFileIOEvent::EVT_FILE_SEEKOFFSET, pJob, pEvent));
}

