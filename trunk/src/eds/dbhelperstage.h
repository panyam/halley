//*****************************************************************************
/*!
 *  \file   dbhelperstage.h
 *
 *  \brief  The stage that handles DB connections and queries
 *  asynchronously.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SDBHELPER_STAGE_H_
#define _SDBHELPER_STAGE_H_

#include "stage.h"

//*****************************************************************************
/*!
 *  \class  SDBHelperStage
 *
 *  \brief  The stage that handles DB connections and queries
 *  asynchronously.
 *
 *****************************************************************************/
class SDBHelperStage : public SStage
{
public:
    // Creates a new handler
    SDBHelperStage(int numThreads = 0);
    
    // Destroys the stage
    virtual ~SDBHelperStage() { }

    //! Opens a DB connection
    virtual void Connect(SJob *pJob, const SString &dbURI, const std::map<SString, void *> &params);

    //! Runs a query
    virtual void RunQuery(SJob *pJob, const SString &query);

    //! Disconnect from a database
    virtual void Disconnect(SJob *pJob, void *pConnection);
};

#endif

