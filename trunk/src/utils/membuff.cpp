//*****************************************************************************
/*!
 *  \file   membuff.cpp
 *
 *  \brief
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include <string.h>
#include "utils/membuff.h"

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
SMemBuff::SMemBuff(unsigned capacity) :
    pBuffer(0),
    pLength(0)
{
    EnsureCapacity(capacity);
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
SMemBuff::~SMemBuff()
{
    if (pBuffer)
        delete [] pBuffer;
}

//*****************************************************************************
/*!
 *  \brief  Ensure's capacity of the write buffer.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SMemBuff::EnsureCapacity(unsigned capacity)
{
    if (pBuffer == 0)
    {
        pBuffer     = new char[capacity];
        pCapacity   = capacity;
        pLength     = 0;
    }
    else if (capacity > pCapacity)
    {
        pCapacity = capacity > (pCapacity * 2) ? capacity : pCapacity * 2;
        char *pOld  = pBuffer;
        pBuffer     = new char[pCapacity];
        memcpy(pBuffer, pOld, pLength);
        delete [] pOld;
    }
    return pCapacity;
}

//*****************************************************************************
/*!
 *  \brief  Writes data to the buffer.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
void SMemBuff::Write(const char *pSrc, size_t len)
{
    if (len > 0 && pSrc)
    {
        EnsureCapacity(pLength + len + 1);
        memcpy(pBuffer + pLength, pSrc, len);
        pLength += len;
    }
}

//*****************************************************************************
/*!
 *  \brief  Reads data to a buffer.
 *
 *  \version
 *      - Sri Panyam      04/03/2009
 *        Created.
 *
 *****************************************************************************/
int SMemBuff::Read(char *buffer, size_t len)
{
    return 0;
}

