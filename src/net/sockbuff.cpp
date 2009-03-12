//*****************************************************************************
/*!
 *  \file   sockbuff.cpp
 *
 *  \brief
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "net/sockbuff.h"

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
SSocketBuff::SSocketBuff(int sock) : sockHandle(sock)
{
    // set up the output buffer to leave at least one space empty:
    setp(pWriteBuffer, pWriteBuffer + BUFFER_SIZE - 1);
}

//*****************************************************************************
/*!
 *  \brief  Called when chars are output
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
SSocketBuff::int_type SSocketBuff::overflow(int_type c)
{
    if (!traits_type::eq_int_type(traits_type::eof(), c))
    {
        traits_type::assign(*pptr(), traits_type::to_char_type(c));
        pbump(1);
    }

    return sync() == 0 ? traits_type::not_eof(c) : traits_type::eof();
}

//*****************************************************************************
/*!
 *  \brief  Flushes the output buffer.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SSocketBuff::sync()
{
    // nothing to transmit so return success
    if (pbase() == pptr())
        return 0;

    size_t count    = pptr() - pbase();
    size_t offset   = 0;
    int numWritten  = 0;

    while (count > 0)
    {
        numWritten = send(sockHandle, pbase() + offset, count, MSG_NOSIGNAL);
        if (numWritten < 0)
        {
            cerr << "Write Error: " << errno << " - " << strerror(errno) << endl;

            // TODO: We are currently clearing out things we arent sending,
            // should we cache it somehow depending on the error?
            // count   =   0;
            // offset  +=  count;
            return -1;
        }
        count -= numWritten;
        offset += numWritten;
    }

    // reset the write buffer
    setp(pWriteBuffer, pWriteBuffer + BUFFER_SIZE - 1);

    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Called when chars are read
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
SSocketBuff::int_type SSocketBuff::underflow()
{
    int rc = recv(sockHandle, pReadBuffer, BUFFER_SIZE, 0);

    if (rc <= 0)
        return traits_type::eof();

    setg(pReadBuffer, pReadBuffer, pReadBuffer + rc);

    return traits_type::to_int_type(*gptr());
}

