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
 *  \brief  The stage that asynchronously reads messages of the socket.
 *
 *  \version
 *      - S Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SREADER_STAGE_H_
#define _SREADER_STAGE_H_

#include "eds/stage.h"

//*****************************************************************************
/*!
 *  \class  SReaderStage
 *
 *  \brief  A generic stage that reads messages of a connection.
 *
 *****************************************************************************/
class SReaderStage : public SStage
{
public:
    // Allowed events in this stage
    typedef enum
    {
        EVT_READ_REQUEST,
        EVT_CLOSE_CONNECTION
    } EventType;

public:
    // Creates a new fileio helper stage
    SReaderStage(const SString &name = "Reader", int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual         ~SReaderStage();

    //! Called when data is available to be read.
    virtual bool    SendEvent_ReadRequest(SConnection *pConnection);

    //! Just deals with read_request event
    virtual void    HandleReadRequestEvent(const SEvent &event);

protected:
    //! Does the actual event handling.
    virtual void    HandleEvent(const SEvent &event);

    //! Tries to assemble the request object from a byte buffer
    virtual void *  AssembleRequest(char *&pStart, char *&pLast, void *pState) { return NULL; }

    //! Handles the newly assembled request
    virtual bool    HandleRequest(SConnection *pConnection, void *pRequest) { return true; }
};

#endif

