//*****************************************************************************
/*!
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
    virtual void Reset() { boundaries.clear(); }

public:
    //! A stack that maintains the boundaries we have opened so far (for
    // multipart messages).
    std::list<std::string>  boundaries;

    //! Here is where we accumulate body parts that are less than
    // content lengths in single part messages
    std::vector<char>   bodyAccumulator;
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

protected:
    void HandleBodyPart(SHttpHandlerData *  pHandlerData, 
                        SHttpHandlerStage * pStage,
                        SContentModuleData *pModData,
                        SBodyPart *         pBodyPart);
};

#endif

