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
 *  \file   handlerstage.h
 *
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_HANDLER_STAGE_H_
#define _SHTTP_HANDLER_STAGE_H_

#include "eds/stage.h"
#include "httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SHttpHandlerStage
 *
 *  \brief  The stage that reads http requests off the socket.
 *
 *****************************************************************************/
class SHttpHandlerStage : public SStage
{
public:
    typedef enum
    {
        EVT_REQUEST_ARRIVED = 0,
        EVT_NEXT_INPUT_MODULE,
        EVT_INPUT_BODY_TO_MODULE,
        EVT_NEXT_OUTPUT_MODULE,
        EVT_OUTPUT_BODY_TO_MODULE,
    } SHandlerEvent;

public:
    // Creates a new fileio helper stage
    SHttpHandlerStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual ~SHttpHandlerStage() { }

    //! Set the reader stage
    virtual void    SetReaderStage(SHttpReaderStage *pReader) { pReaderStage = pReader; }

    //! Get the reader stage
    virtual SHttpReaderStage *GetReaderStage() { return pReaderStage; }

    //! Set the Writer stage
    virtual void    SetWriterStage(SHttpWriterStage *pWriter) { pWriterStage = pWriter; }

    //! Get the Writer stage
    virtual SHttpWriterStage *GetWriterStage() { return pWriterStage; }

    //! Sets the root processing module.
    virtual void SetRootModule(SHttpModule *pModule) { pRootModule = pModule; }
    
    //! Sends input to be processed by a module
    virtual bool SendEvent_InputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart = NULL);
    
    //! Sends output to be processed by a module
    virtual bool SendEvent_OutputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart = NULL);

    //! Handle a new request
    virtual bool SendEvent_HandleNextRequest(SConnection *pConnection, SHttpRequest *pRequest);
    
    //! Sends body part to be written out
    virtual bool SendEvent_WriteBodyPart(SConnection *pConnection, SHttpRequest *pRequest, SBodyPart *pBodyPart);

    //! Called when a job is destroyed
    virtual void JobDestroyed(SJob *pJob);

protected:
    //! Handle the actual event
    virtual void HandleEvent(const SEvent &event);

protected:
    //! The stage that reads requests from the connection
    SHttpReaderStage *      pReaderStage;

    //! The stage that writes requests to the connection
    SHttpWriterStage *      pWriterStage;

    //! The starting module for all requests
    SHttpModule *           pRootModule;
};

#endif

