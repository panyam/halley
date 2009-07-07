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
 *  \file   readerstage.h
 *
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_READER_STAGE_H_
#define _SHTTP_READER_STAGE_H_

#include "eds/stage.h"
#include "httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SHttpReaderStage
 *
 *  \brief  The stage that reads http requests off the socket.
 *
 *****************************************************************************/
class SHttpReaderStage : public SStage
{
public:
    // Allowed events in this stage
    typedef enum
    {
        EVT_BYTES_RECIEVED = 0,
    } EventType;

public:
    // Creates a new fileio helper stage
    SHttpReaderStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual ~SHttpReaderStage() { }

    //! Set the handler stage
    virtual void    SetHandlerStage(SHttpHandlerStage *pHandler);

    //! Get the handler stage
    virtual SHttpHandlerStage *GetHandlerStage() { return pHandlerStage; }

    //! Called when bytes are available
    void    ReadSocket(SConnection *pConnection);

    //! Called when a job is destroyed
    virtual void JobDestroyed(SJob *pJob);

protected:
    //! Does the actual event handling.
    virtual void HandleEvent(const SEvent &event);

protected:
    //! The stage that actually handles a fully read request
    SHttpHandlerStage *pHandlerStage;
};

#endif

