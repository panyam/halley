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
 *  \file   logger.cpp
 *
 *  \brief  Central place for all logging utilities.
 *
 *  \version
 *      - Sri Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#include "logger.h"

//*****************************************************************************
/*!
 *  \brief  Gets a logger by index/purpose.
 *
 *  \version
 *      - Sri Panyam     08/07/2009
 *        Created.
 *
 *****************************************************************************/
SLogger *SLogger::Get(int index)
{
    return loggers[index];
}

//*****************************************************************************
/*!
 *  \brief  Adds a new logger to our list.
 *
 *  \version
 *      - Sri Panyam     08/07/2009
 *        Created.
 *
 *****************************************************************************/
int SLogger::Add(SLogger *pLogger)
{
    // are duplicates allowed?
    loggers.push_back(pLogger);
    return loggers.size() - 1;
}

