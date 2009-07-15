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
 *
 *  Miscellaneous utilities.
 *
 *  \version
 *      - Sri Panyam      15/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEDS_UTILS_H_
#define _SEDS_UTILS_H_

#include "fwd.h"

//*****************************************************************************
/*!
 *  \class  SEDSUtils
 *
 *  \brief Class for miscellaneous utilities.
 *****************************************************************************/
class SEDSUtils
{
public:
    //! Sends data over a fd completely
    static int SendFully(int fd, const char *buffer, int length);

    //! Sets/Clears nonblocking mode on a FD
    static int SetNonBlocking(int fd, bool set = true);
};

#endif

