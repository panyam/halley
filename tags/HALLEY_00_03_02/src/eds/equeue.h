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
 *  \file   equeue.h
 *
 *  \brief  The event queue class.  This is one of the things that needs to
 *  be optimised to allow for high concurrency.  For now we will use a
 *  simple priority queue.
 *
 *  \version
 *      - S Panyam      06/07/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_QUEUE_H_
#define _SEVENT_QUEUE_H_

#include <queue>
#include "eds/event.h"

//*****************************************************************************
/*!
 *  \class  SEventQueue
 *
 *  \brief  The event queue.
 *
 *****************************************************************************/
class SEventQueue : public std::priority_queue<SEvent>
{
};

#endif

