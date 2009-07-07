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
SSockWriterStage::SSockWriterStage(const SString &name, int numThreads) : SStage(name, numThreads)
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

