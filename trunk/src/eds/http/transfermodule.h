//*****************************************************************************
/*!
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
    virtual void ProcessOutput(SHttpHandlerData *   pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart);

protected:
    void HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SHttpModuleData *   pModData,
                        SBodyPart *         pBodyPart);
};

#endif

