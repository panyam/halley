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
 *  \file   writermodule.h
 *
 *  \brief  A module that writes the request to the network - this MUST be
 *  the last stage in a chain.
 *
 *  \version
 *      - S Panyam      11/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SWRITER_MODULE_H_
#define _SWRITER_MODULE_H_

#include "httpmodule.h"

// Takes care of transfer encoding - Strips out Content-Length in chunked
// mode
class SWriterModule : public SHttpModule
{
public:
    //! Create it
    SWriterModule() : SHttpModule(NULL) { }

    //! Called to handle output data from another module
    virtual void ProcessOutput(SHttpHandlerData *   pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart);

protected:
    bool HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SHttpModuleData *   pModData,
                        SBodyPart *         pBodyPart, 
                        std::ostream &      outStream);
};

#endif

