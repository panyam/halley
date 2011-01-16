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
 *  \file   urlutils.h
 *
 *  \brief
 *  A HTTP utilities.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef URL_UTILS_H
#define URL_UTILS_H

#include <string>
#include <string.h>
#include <sstream>

//! Generic URL utilities 
class URLUtils
{
public:
    const static char   CR;
    const static char   LF;
    const static char * CRLF;
    const static char * SEPARATORS;

    static inline bool isseperator(char ch)     { return  (strchr(SEPARATORS, (ch)) != 0); }
    static inline bool iscontrol(char ch)       { return ( ((ch) == 127) || (((ch) >= 0) && ((ch) <= 31))); }

public:
    static std::string ReadTillCrLf(std::istream &input);
    static std::string base64_encode(unsigned char const* , unsigned int len);
    static std::string base64_decode(std::string const& s);
    static std::string Escape(const std::string &);
    static std::string Unescape(const std::string &);
    static bool ExtractNextQuery(const char *&queryString, std::string &qName, std::string &qValue);
};


#endif

