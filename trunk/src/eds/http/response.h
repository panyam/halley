//*****************************************************************************
/*!
 *  \file   httpresponse.h
 *
 *  \brief
 *  Object to store a http response.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_RESPONSE_H_
#define _SHTTP_RESPONSE_H_

#include <map>
#include <string>
#include <iostream>

#include "message.h"

//*****************************************************************************
/*!
 *  \class  SHttpResponse
 *
 *  \brief  A http response object.
 *
 *****************************************************************************/
class SHttpResponse : public SHttpMessage
{
public:
    SHttpResponse();
    virtual         ~SHttpResponse();

public:
    //! Sets the response status
    int StatusCode() const;

    //! The status message
    const SString StatusMessage() const;

    //! Sets the response status
    void SetStatus(int status, const SString &msg);

    //! Write the response to a stream.
    virtual bool WriteToStream(std::ostream &output);

protected:
    //! Reads the first status line
    // virtual bool ReadFirstLine(std::istream &input);

protected:
    //! Server's error status, eg 200, 404 etc
    int                                 statusCode;

    //! Status message eg OK, Not Found etc
    SString                         statusMessage;
};

#endif

