
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

