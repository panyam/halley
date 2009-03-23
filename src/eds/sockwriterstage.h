//*****************************************************************************
/*!
 *  \file   sockwriter.h
 *
 *  \brief
 *
 *  A writer of sockets in nonblocking mode.  As data needs to be written
 *  out to sockets, we cannot block on them.  So we let this thread take
 *  care queueing all these write requests so they can be written slowly.
 *  Also a callback can be called after this data is written.
 *
 *  \version
 *      - Sri Panyam      23/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SOCKET_WRITER_STAGE_H_
#define _SOCKET_WRITER_STAGE_H_

#include "stage.h"

class SSockWriterData
{
public:
    //! Constructor
    SSockWriterData(char *data, size_t len) : pData(data), length(len)  { }

    const char *    pData;
    size_t          length;
};

//*****************************************************************************
/*!
 *  \class  SDBHelperStage
 *
 *  \brief  The stage that handles DB connections and queries
 *  asynchronously.
 *
 *****************************************************************************/
class SSockWriterStage : public SStage
{
public:
    // Creates a new handler
    SSockWriterStage(int numThreads = 0);
    
    // Destroys the stage
    virtual ~SSockWriterStage() { }

    //! Opens a DB connection
    virtual void WriteData(SJob *pJob, int sockHandle, char *data, size_t length);

    //! Runs a query
    virtual void RunQuery(SJob *pJob, const std::string &query);
};

#endif


