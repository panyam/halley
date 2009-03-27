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
 *  \class  defaultconnfactory.h
 *
 *  \brief  A default connection factory.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef SDEFAULT_CONN_FACTORY_H
#define SDEFAULT_CONN_FACTORY_H

#include "connfactory.h"

//*****************************************************************************
/*!
 *  \class  SDefaultConnFactory
 *
 *  \brief  A factory for creating and destorying connection handlers.
 *
 *****************************************************************************/
template <class T>
class SDefaultConnFactory : public SConnFactory
{
public:
    virtual SConnHandler *  NewHandler() { return new T(); }
    virtual void            ReleaseHandler(SConnHandler * handler) { delete handler; }
};

#endif

