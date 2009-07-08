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
        EVT_BYTES_RECIEVED = 0,
    } EventType;

public:
    // Creates a new fileio helper stage
    SReaderStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual         ~SReaderStage();

    //! Set the handler stage
    virtual void    SetHandlerStage(SStage *pHandler) { pHandlerStage = pHandler; }

    //! Get the handler stage
    virtual SStage *GetHandlerStage() { return pHandlerStage; }

    //! Called when data is available to be read.
    virtual void    ReadSocket(SConnection *pConnection);

    //! Called when a job is destroyed
    virtual void    JobDestroyed(SJob *pJob);

protected:
    //! Does the actual event handling.
    virtual void    HandleEvent(const SEvent &event);

    //! Creates the protocol specific reader state object
    virtual void *  CreateReaderState() { return NULL; }

    //! Destroys the the protocol specific reader state object
    virtual void    DestroyReaderState(void *pStateData) { }

    //! Tries to assemble the request object from a byte buffer
    virtual void *  AssembleRequest(char *&pStart, char *&pLast, void *pState) { return NULL; }

    //! Current read buffer - to store "Extra" data that may be remaining
    char *          pReadBuffer;
    char *          pCurrPos;
    char *          pBuffEnd;

private:
    //! The request handler stage which handles assembled requests
    SStage *        pHandlerStage;
};

#endif

