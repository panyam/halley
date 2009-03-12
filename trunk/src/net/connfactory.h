
#ifndef SCONN_FACTORY_H
#define SCONN_FACTORY_H

//*****************************************************************************
/*!
 *  \class  SConnFactory
 *
 *  \brief  A factory for creating and destorying connection handlers.
 *
 *****************************************************************************/
class SConnFactory
{
public:
    virtual ~SConnFactory() { }
    virtual SConnHandler *  NewHandler()                            = 0;
    virtual void            ReleaseHandler(SConnHandler * handler)  = 0;
};

#endif

