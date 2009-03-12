//*****************************************************************************
/*!
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

