//*****************************************************************************
/*!
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
    void HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SHttpModuleData *   pModData,
                        SBodyPart *         pBodyPart, 
                        std::ostream &      outStream);
};

#endif

