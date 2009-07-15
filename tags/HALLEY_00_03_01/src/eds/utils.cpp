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
 *  \brief  Miscellaneous utilities.
 *
 *  \version
 *      - Sri Panyam      15/07/2009
 *        Created
 *
 *****************************************************************************/

#include "utils.h"
#include <sys/socket.h>

//! Sends data over a fd completely
int SEDSUtils::SendFully(int fd, const char *buffer, int length)
{
    while (length > 0)
    {
        int numWritten = send(fd, buffer, length, MSG_NOSIGNAL);
        if (numWritten < 0)
        {
            return -1;
        }
        else
        {
            buffer += numWritten;
            length -= numWritten;
        }
    }
    return length;
}

