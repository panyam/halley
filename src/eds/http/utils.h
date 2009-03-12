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

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>

#define CR                  '\r'
#define LF                  '\n'
#define CRLF                "\r\n"
#define SEPARATORS          "()<>@,;:\\\"/[]?={} \t"
#define isseperator(ch)     (strchr(SEPARATORS, (ch)) != 0)
#define iscontrol(ch)       ( ((ch) == 127) || (((ch) >= 0) && ((ch) <= 31)))

std::string ReadTillCrLf(std::istream &input);

#endif

