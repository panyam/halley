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
 *  \file   sockbuff.h
 *
 *  \brief  IO Stream for TCP Sockets
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SSOCKSTREAM_H_
#define _SSOCKSTREAM_H_

#include <iostream>
#include <string>
#include <streambuf>
using namespace std;

//*****************************************************************************
/*!
 *  \class  SSocketBuff
 *
 *  \brief  A iostream wrapper for tcp sockets
 *
 *****************************************************************************/
class SSocketBuff : public std::streambuf
{
public:
    SSocketBuff(int sockHandle, size_t bsize = DEFAULT_BUFFER_SIZE + 1);
    ~SSocketBuff();

protected:
    int_type    overflow(int_type c);
    int_type    underflow();
    int         sync();

protected:
    enum { DEFAULT_BUFFER_SIZE    = (1 << 15) };
    int     sockHandle;
    size_t  buffSize;
    char    *pReadBuffer;
    char    *pWriteBuffer;
};

#endif

