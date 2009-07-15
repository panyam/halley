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
 *  \file   writerstage.cpp
 *
 *  \brief  The stage that asynchronously reads requests/messages of the
 *  socket - this can be inherited to read protocol specific messages.
 *
 *  \version
 *      - S Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#include "connection.h"
#include "writerstage.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <iostream>

// Creates a message writer stage.
SWriterStage::SWriterStage(const SString &name, int numThreads) : SStage(name, numThreads)
{
}

//! Destroys writer data
SWriterStage::~SWriterStage()
{
}

//! Initiates another load of data writing - called by the server instead
// of other stages
void SWriterStage::SendEvent_WriteData(SConnection *pConnection)
{
    QueueEvent(SEvent(EVT_WRITE_DATA, pConnection));
}

//! Handles "read request" events.
//
// Will call the RequestHandler stage when a complete request has been read.
//
// The previous model was to read requests and queue them in.
void SWriterStage::HandleEvent(const SEvent &event)
{
    // The connection currently being processed
}

