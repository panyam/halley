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
 *  \brief  The stage that asynchronously writes messages on to the socket.
 *
 *  \version
 *      - S Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SWRITER_STAGE_H_
#define _SWRITER_STAGE_H_

#include "eds/stage.h"

//*****************************************************************************
/*!
 *  \class  SWriterStage
 *
 *  \brief  A generic stage that writes messages onto a connection.
 *
 *****************************************************************************/
class SWriterStage : public SStage
{
public:
    enum
    {
        EVT_WRITE_DATA
    };

public:
    // Creates a new writer stage
    SWriterStage(const SString &name, int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual         ~SWriterStage();

    // Initiates another load of writing
    void            SendEvent_WriteData(SConnection *pConnection);

protected:
    //! Does the actual event handling.
    virtual void    HandleEvent(const SEvent &event);
};

#endif

