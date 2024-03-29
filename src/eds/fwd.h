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
 *  \file   fwd.h
 *
 *  \brief
 *  Forward defs for EDS module.
 *
 *  \version
 *      - Sri Panyam      20/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef EDS_FWD_H
#define EDS_FWD_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <queue>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <iomanip>
#include <iostream>
#include "logger/logger.h"

class SJob;
class SJobListener;
class SConnection;
class SBodyPart;

class SEvent;
class SEventDispatcher;
class SEventHandler;
class SEventHandlerFactory;
class SEventQueue;
class SEvServer;

class SStage;
class SReaderStage;
class SWriterStage;

typedef std::string                     SString;
typedef std::stringstream               SStringStream;
typedef std::vector<char>               SCharVector;
typedef std::list<SConnection *>        SConnectionList;
typedef std::list<SString>              SStringList;
typedef std::vector<SString>            SStringVector;
typedef std::pair<SString, SString>     SStringPair;

// shortcut for a connection list and set
typedef std::list<SConnection *>        TConnectionList;
typedef std::set<SConnection *>         TConnectionSet;

//! A type to denote body part indices.
typedef unsigned BPIndexType;

#endif

