//*****************************************************************************
/*!
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
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <queue>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <iomanip>
#include <iostream>

class SJob;
class SConnection;

typedef std::vector<char>                   SCharVector;
typedef std::list<SConnection *>            SConnectionList;
typedef std::list<std::string>              SStringList;
typedef std::vector<std::string>            SStringVector;
typedef std::pair<std::string, std::string> SStringPair;

class SEvent;
class SEventDispatcher;
class SEventHandler;
class SEventHandlerFactory;
class SEvServer;

class SStage;
class SFileIOHelper;
class SFileIOStage;
class SDBHelperStage;

#endif

