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
 *  \file   logger.h
 *
 *  \brief  Central place for all logging utilities.
 *
 *  \version
 *      - Sri Panyam      08/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SLOGGER_H_
#define _SLOGGER_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//*****************************************************************************
/*!
 *  \class  SLogger
 *
 *  \brief  Super class of all loggers.
 *
 *****************************************************************************/
class SLogger
{
public:
    // Constructor
    SLogger() { }

    //! virtual destructor
    virtual ~SLogger() { }

    //! Does the actual logging
    virtual int Log(int level, const char *fmt, ...);

    //! Get a specific logger
    static SLogger *Get(int index = 0);

    //! Adds a new logger to our list
    static int Add(SLogger *pLogger);

private:
    //! A map of all loggers
    static std::vector<SLogger *>    loggers;
};

#endif

