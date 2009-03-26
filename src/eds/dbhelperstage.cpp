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
void SDBHelperStage::Connect(void *pSource, const SString &dbURI, const std::map<SString, void *> &params)
{
}

//! Runs a query
void SDBHelperStage::RunQuery(void *pSource, const SString &query)
{
}

//! Disconnect from a database
void SDBHelperStage::Disconnect(void *pSource, void *pConnection)
{
}
