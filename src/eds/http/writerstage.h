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
 *  \file   writerstage.h
 *
 *  \brief  A stage that writes the request to the socket.  The modules
 *  dont/shouldnt care about this.  When there is no longer a module to
 *  send to, the http handler stage will automatically send the data to
 *  this stage.
 *
 *  \version
 *      - S Panyam      14/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_WRITER_STAGE_H_
#define _SHTTP_WRITER_STAGE_H_

#include "eds/writerstage.h"

class SBodyPart;
class SHttpStageData;

// Takes care of transfer encoding - Strips out Content-Length in chunked
// mode
class SHttpWriterStage : public SWriterStage
{
public:
    // Allowed events in this stage
    typedef enum
    {
        EVT_WRITE_BODY_PART = 1,    // from 1 since parent uses 0
    } EventType;

public:
    // Creates a http request writer
    SHttpWriterStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);

    // Destroys the stage
    virtual ~SHttpWriterStage() { }

    //! Set the reader stage
    virtual void    SetReaderStage(SHttpReaderStage *pReader) { pReaderStage = pReader; }

    //! Get the reader stage
    virtual SHttpReaderStage *GetReaderStage() { return pReaderStage; }

    //! Creates the stage specific object
    virtual void *  CreateStageData();

    //! Destroys the stage specific object
    virtual void    DestroyStageData(void *pStateData);

    //! Re orders and sends out http body parts to the socket
    virtual void HandleEvent(const SEvent &event);

protected:
    // Handles each body part writing
    bool HandleBodyPart(SConnection *       pConnection,
                        SHttpStageData *    pStageData,
                        SBodyPart *         pBodyPart,
                        std::ostream &      outStream);

protected:
    //! The stage that reads requests from the connection
    SHttpReaderStage *      pReaderStage;
};

#endif

