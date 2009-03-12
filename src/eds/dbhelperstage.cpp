//*****************************************************************************
/*!
 *  \file   dbhelperstage.cpp
 *
 *  \brief  The stage that does asynchronous DB Connections and Queries.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#include "dbhelperstage.h"

// Creates a new file io helper stage
SDBHelperStage::SDBHelperStage(int numThreads) : SStage(numThreads)
{
}


//! Opens a DB connection
void SDBHelperStage::Connect(SJob *pJob, const std::string &dbURI, const std::map<std::string, void *> &params)
{
}

//! Runs a query
void SDBHelperStage::RunQuery(SJob *pJob, const std::string &query)
{
}

//! Disconnect from a database
void SDBHelperStage::Disconnect(SJob *pJob, void *pConnection)
{
}
