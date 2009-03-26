//*****************************************************************************
/*!
 *  \file   sockwriterstage.cpp
 *
 *  \brief
 *  A writer of sockets in nonblocking mode.  As data needs to be written
 *  out to sockets, we cannot block on them.  So we let this thread take
 *  care queueing all these write requests so they can be written slowly.
 *  Also a callback can be called after this data is written.
 *
 *  \version
 *      - S Panyam      23/03/2009
 *        Created
 *
 *****************************************************************************/

#include "sockwriterstage.h"

// Creates a new file io helper stage
SSockWriterStage::SSockWriterStage(int numThreads) : SStage(numThreads)
{
}


//! Queues data to be written to a socket
void SSockWriterStage::WriteData(void *pSource, int sockHandle, char *data, size_t length)
{
}

//! Runs a query
void SSockWriterStage::RunQuery(void *pSource, const SString &query)
{
}

