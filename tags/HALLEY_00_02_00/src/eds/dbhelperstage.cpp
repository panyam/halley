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
