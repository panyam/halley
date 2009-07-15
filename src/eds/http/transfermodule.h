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
 *  \file   transfermodule.h
 *
 *  \brief  A module that handles all transfer encodings.
 *
 *  \version
 *      - S Panyam      10/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _STRANSFER_MODULE_H_
#define _STRANSFER_MODULE_H_

#include "httpmodule.h"

// Takes care of transfer encoding - Strips out Content-Length in chunked
// mode
class STransferModule : public SHttpModule
{
public:
    //! Create it
    STransferModule(SHttpModule *pNext) : SHttpModule(pNext) { }

    //! Called to handle output data from another module
    virtual void ProcessOutput(SConnection *        pConnection,
                               SHttpHandlerData *   pHandlerData,
                               SHttpHandlerStage *  pStage,
                               SBodyPart *          pBodyPart);

protected:
    void HandleBodyPart(SConnection *       pConnection,
                        SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SHttpModuleData *   pModData,
                        SBodyPart *         pBodyPart);
};

#endif

