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
 *  \file   evhandler.h
 *
 *  \brief  Superclass of all tasks that simply handle events.
 *
 *  \version
 *      - S Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEVENT_HANDLER_H_
#define _SEVENT_HANDLER_H_

#include "event.h"

//*****************************************************************************
/*!
 *  \class  SEventHandler
 *
 *  \brief  A Handler of events.  Does the real task of processing the
 *  events.
 *
 *****************************************************************************/
class SEventHandler
{
public:
    //! Creates a new handler
    SEventHandler(SStage *pStage_) : pStage(pStage) { }

    // Creates a new handler
    virtual ~SEventHandler() { }

    //! The actual handler function.
    virtual void HandleEvent(const SEvent &event)  = 0;

    //! Get the stage which this handler belongs to.
    SStage *GetStage() { return pStage; }

protected:
    //! Parent stage to which this handler belongs
    SStage *pStage;

private:
    //! Defeat instantiation or assignment
    SEventHandler(const SEventHandler &);
    SEventHandler & operator=(const SEventHandler &);
};

//*****************************************************************************
/*!
 *  \class  SEventHandlerFactory
 *
 *  \brief  A factory for creating the handlers.  These are required
 *  because each stage handles different kinds of events and each stage is
 *  for a homogeneous class of events.
 *
 *****************************************************************************/
class SEventHandlerFactory
{
public:
    //! Destroys the factory
    virtual ~SEventHandlerFactory() { }

    //! Create a new handler 
    virtual SEventHandler *  NewHandler(SStage *pStage) = 0;

    //! destroyes a handler
    virtual void ReleaseHandler(SEventHandler *pHandler) = 0;
};

template <class T>
class SDefaultEventHandlerFactory : public SEventHandlerFactory
{
public:
    //! Destroys the factory
    virtual ~SDefaultEventHandlerFactory() { }

    //! Create a new handler 
    virtual SEventHandler *  NewHandler(SStage *pStage) { return new T(pStage); }

    //! destroyes a handler
    virtual void ReleaseHandler(SEventHandler *pHandler) { delete pHandler; }
};

#endif

