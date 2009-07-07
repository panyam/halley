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
    SDBHelperStage(int numThreads = DEFAULT_NUM_THREADS);
    
    // Destroys the stage
    virtual ~SDBHelperStage() { }

    //! Opens a DB connection
    virtual void Connect(void *pSource, const SString &dbURI, const std::map<SString, void *> &params);

    //! Runs a query
    virtual void RunQuery(void *pSource, const SString &query);

    //! Disconnect from a database
    virtual void Disconnect(void *pSource, void *pConnection);
};

#endif

