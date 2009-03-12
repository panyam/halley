//*****************************************************************************
/*!
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
    SSocketBuff(int sockHandle);

protected:
    int_type    overflow(int_type c);
    int_type    underflow();
    int         sync();

protected:
    enum { BUFFER_SIZE    = (1 << 15) };
    int     sockHandle;
    char    pReadBuffer[BUFFER_SIZE + 1];
    char    pWriteBuffer[BUFFER_SIZE + 1];
};

#endif

