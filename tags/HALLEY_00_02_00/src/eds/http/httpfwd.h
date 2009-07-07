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

#include "../fwd.h"

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
class SBayeuxModule;
class SBayeuxChannel;
class SChannelListener;

#endif
