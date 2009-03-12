//*****************************************************************************
/*!
 *  \file   httpfwd.h
 *
 *  \brief
 *  Forward defs for EDS HTTP module.
 *
 *  \version
 *      - Sri Panyam      04/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef HTTP_FWD_H
#define HTTP_FWD_H

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>

//! A type to denote body part indices.
typedef unsigned BPIndexType;

class SMemBuff;
class SBodyPart;
class SHeaderTable;
class SHttpRequest;
class SHttpResponse;
class SHttpModule;
class SHttpHandlerData;
class SHttpReaderStage;
class SHttpHandlerStage;

#endif

