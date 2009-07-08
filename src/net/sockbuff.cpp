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
 *  \file   sockbuff.cpp
 *
 *  \brief  Stream buffer for sockets.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/epoll.h>
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
SSocketBuff::SSocketBuff(int sock, size_t bsize) :
    sockHandle(sock),
    buffSize(bsize),
    pReadBuffer(new char[buffSize + 1]),
    pWriteBuffer(new char[buffSize + 1])
{
    // set up the output buffer to leave at least one space empty:
    setp(pWriteBuffer, pWriteBuffer + buffSize - 1);
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam      23/03/2009
 *        Created.
 *
 *****************************************************************************/
SSocketBuff::~SSocketBuff()
{
    if (pReadBuffer) delete [] pReadBuffer;
    if (pWriteBuffer) delete [] pWriteBuffer;
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
        numWritten = -1; //send(sockHandle, pbase() + offset, count, MSG_NOSIGNAL);
        errno = EAGAIN;
        if (numWritten < 0)
        {
            // block till written! - not good need to palm this off to
            // another thread - but how do we handle memory for this
            // block?
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // create structures to allow async writing
                int kdpfd   = epoll_create(1);
                int currfds = 1;
                struct epoll_event ev;
                struct epoll_event events[1];
                bzero(&ev, sizeof(ev));
                ev.events   = EPOLLOUT | EPOLLET;
                ev.data.fd  = sockHandle;
                if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, sockHandle, &ev) < 0)
                {
                    std::cerr << "WRITE ERROR: epoll_ctl error: [" << errno << "]: " << strerror(errno) << std::endl;
                    close(kdpfd);
                    return -1;
                }

                while (count > 0)
                {
                    int nfds = epoll_wait(kdpfd, events, currfds, -1);
                    if (nfds < 0)
                    {
                        std::cerr << "WRITE ERROR: epoll_wait error: [" << errno << "]: " << strerror(errno) << std::endl;
                        close(kdpfd);
                        return -1;
                    }

                    assert("Too many fds found!!!" && nfds == 1);

                    numWritten = send(sockHandle, pbase() + offset, count, MSG_NOSIGNAL);
                    if (numWritten < 0)
                    {
                        std::cerr << "WRITE ERROR: send error: [" << errno << "]: " << strerror(errno) << std::endl;
                        close(kdpfd);
                        return -1;
                    }
                    count -= numWritten;
                    offset += numWritten;
                }
                close(kdpfd);
            }
            else
            {
                std::cerr << "WRITE ERROR: send error: [" << errno << "]: " << strerror(errno) << std::endl;
                return -1;
            }
        }
        else
        {
            count -= numWritten;
            offset += numWritten;
        }
    }

    // reset the write buffer
    setp(pWriteBuffer, pWriteBuffer + buffSize - 1);

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
    int rc = recv(sockHandle, pReadBuffer, buffSize, 0);

    if (rc <= 0)
        return traits_type::eof();

    setg(pReadBuffer, pReadBuffer, pReadBuffer + rc);

    return traits_type::to_int_type(*gptr());
}

