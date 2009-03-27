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
 *  \file   membuff.h
 *
 *  \brief  An output stream that prepends the "size" of the data it sends
 *          on each request.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _MEMBUFF_H_
#define _MEMBUFF_H_

#include <stdio.h>
#include <stdlib.h>

//*****************************************************************************
/*!
 *  \class  SMemBuff
 *
 *  \brief  An output memory buffer (can be extended to be an input buffer
 *  too!)
 *
 *****************************************************************************/
class SMemBuff
{
public:
                    SMemBuff(unsigned capacity = 1024);
                    ~SMemBuff();
    int             EnsureCapacity(unsigned capacity);
    inline size_t   Length() const { return pLength; }
    inline size_t   Capacity() const { return pCapacity; }
    const char *    Data() const { return pBuffer; }
    void            Write(const unsigned char *src, size_t len) { Write((const char *)src, len); }
    void            Write(const char *src, size_t len);
    int             Read(char *buffer, size_t len);

protected:
    // The temporary read buffer
    char    *   pBuffer;

    // content size
    size_t      pLength;

    // Capacity of the write buffer
    size_t      pCapacity;
};

#endif

