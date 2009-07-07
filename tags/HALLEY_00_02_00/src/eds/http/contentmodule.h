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
 *  \file   contentmodule.h
 *
 *  \brief  A module that handles all transfer encodings.
 *
 *  \version
 *      - S Panyam      10/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SCONTENT_MODULE_H
#define _SCONTENT_MODULE_H

#include "httpmodule.h"

// Module specific content data - holds stack of opened multipart message
// boundaries.
class SContentModuleData : public SHttpModuleData
{
public:
    //! Destructor
    virtual ~SContentModuleData() { boundaries.clear(); }

    //! Clears boundaries
    virtual void Reset()
    {
        SHttpModuleData::Reset();
        boundaries.clear();
    }

public:
    //! A stack that maintains the boundaries we have opened so far (for
    // multipart messages).
    SStringList     boundaries;
};

// Takes care of content encoding - may or maynot include Content-Length
class SContentModule : public SHttpModule
{
public:
    //! Constructor
    SContentModule(SHttpModule *pNext) : SHttpModule(pNext) { }

    //! Called to handle output data from another module
    virtual void ProcessOutput(SHttpHandlerData *   pHandlerData,
                              SHttpHandlerStage *   pStage, 
                              SBodyPart *           pBodyPart);

    //! Creates new module data if necessary
    virtual SHttpModuleData *CreateModuleData(SHttpHandlerData *pHandlerData)
    {
        return new SContentModuleData();
    }

protected:
    void HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SContentModuleData *pModData,
                        SBodyPart *         pBodyPart);
};

#endif

