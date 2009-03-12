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
 *  \file   utils.cpp
 *
 *  \brief
 *  A HTTP utilities.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include "utils.h"

// Reads a line till the CRLF
std::string ReadTillCrLf(std::istream &input)
{
    std::stringstream out;
    int ch = input.get();
    while (!input.bad() && !input.eof() && ch != CR && ch != LF)
    {
        out << (char)ch;
    	ch = input.get();
    }
    ch = input.get();
    return out.str();
}
