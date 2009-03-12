//*****************************************************************************
/*!
 *  \file   handlerstage.h
 *
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_HANDLER_STAGE_H_
#define _SHTTP_HANDLER_STAGE_H_

#include "eds/stage.h"
#include "httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SHttpHandlerStage
 *
 *  \brief  The stage that reads http requests off the socket.
 *
 *****************************************************************************/
class SHttpHandlerStage : public SStage
{
public:
    typedef enum
    {
        EVT_REQUEST_ARRIVED = 0,
        EVT_NEXT_INPUT_MODULE,
        EVT_INPUT_BODY_TO_MODULE,
        EVT_NEXT_OUTPUT_MODULE,
        EVT_OUTPUT_BODY_TO_MODULE,
        EVT_CLOSE_CONNECTION,
    } SHandlerEvent;

public:
    // Creates a new fileio helper stage
    SHttpHandlerStage(int numThreads = 0);
    
    // Destroys the stage
    virtual ~SHttpHandlerStage() { }

    //! Set the file IO stage
    virtual void    SetIOHelper(SFileIOHelper *pIOHelper);

    //! Set the db helper stage
    virtual void    SetDBHelper(SDBHelperStage *pDBHelper);

    //! Get the file IO stage
    virtual SFileIOHelper *GetIOHelper() { return pIOHelper; }

    //! Get the db helper stage
    virtual SDBHelperStage *GetDBHelper() { return pDBHelper; }

    //! Sets the root processing module.
    virtual void SetRootModule(SHttpModule *pModule) { pRootModule = pModule; }

    //! Handle a new request
    virtual void HandleRequest(SConnection *pConnection, SHttpRequest *pRequest);
    
    //! Sends input to be processed by a module
    virtual void InputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart = NULL);
    
    //! Sends output to be processed by a module
    virtual void OutputToModule(SConnection *pConnection, SHttpModule *pModule, SBodyPart *pBodyPart = NULL);

    //! Request to close the connection
    virtual void CloseConnection(SConnection *pConnection);

protected:
    //! Handle the actual event
    virtual void HandleEvent(const SEvent &event);

protected:
    //! IO Helper
    SFileIOHelper *         pIOHelper;
    
    //! DB Helper
    SDBHelperStage *        pDBHelper;

    //! The starting module for all requests
    SHttpModule *           pRootModule;
};

#endif

