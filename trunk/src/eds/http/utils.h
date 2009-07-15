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
 *  \file   utils.h
 *
 *  \brief
 *  A HTTP utilities.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include "httpfwd.h"
#include "../utils.h"

//! Generic utilities for HTTP
class HttpUtils
{
public:
    const static char   CR;
    const static char   LF;
    const static char * CRLF;
    const static char * SEPARATORS;

    static inline bool isseperator(char ch)     { return  (strchr(SEPARATORS, (ch)) != 0); }
    static inline bool iscontrol(char ch)       { return ( ((ch) == 127) || (((ch) >= 0) && ((ch) <= 31))); }

public:
    static SString ReadTillCrLf(std::istream &input);
    static SString base64_encode(unsigned char const* , unsigned int len);
    static SString base64_decode(SString const& s);
    static SString Escape(const SString &);
    static SString Unescape(const SString &);
};


#endif

