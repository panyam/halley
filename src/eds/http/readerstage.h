//*****************************************************************************
/*!
 *  \file   readerstage.h
 *
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_READER_STAGE_H_
#define _SHTTP_READER_STAGE_H_

#include "eds/stage.h"
#include "httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SHttpReaderStage
 *
 *  \brief  The stage that reads http requests off the socket.
 *
 *****************************************************************************/
class SHttpReaderStage : public SStage
{
public:
    // Allowed events in this stage
    typedef enum
    {
        EVT_BYTES_RECIEVED = 0,
    } EventType;

public:
    // Creates a new fileio helper stage
    SHttpReaderStage(int numThreads = 0);
    
    // Destroys the stage
    virtual ~SHttpReaderStage() { }

    //! Set the handler stage
    virtual void    SetHandlerStage(SHttpHandlerStage *pHandler);

    //! Get the handler stage
    virtual SHttpHandlerStage *GetHandlerStage() { return pHandlerStage; }

    //! Called when bytes are available
    void    ReadSocket(SConnection *pConnection);

protected:
    //! Does the actual event handling.
    virtual void HandleEvent(const SEvent &event);

protected:
    //! The stage that actually handles a fully read request
    SHttpHandlerStage *pHandlerStage;
};

#endif

