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
    SSockWriterStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual ~SSockWriterStage() { }

    //! Opens a DB connection
    virtual void WriteData(void *pSource, int sockHandle, char *data, size_t length);

    //! Runs a query
    virtual void RunQuery(void *pSource, const SString &query);
};

#endif


