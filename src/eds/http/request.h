//*****************************************************************************
/*!
 *  \file   httprequest.h
 *
 *  \brief
 *  Object to store a http request.
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_REQUEST_H_
#define _SHTTP_REQUEST_H_

#include <map>
#include <string>
#include <iostream>

#include "message.h"

//*****************************************************************************
/*!
 *  \class  SHttpRequest
 *
 *  \brief  A http request object.
 *
 *****************************************************************************/
class SHttpRequest : public SHttpMessage
{
public:
    SHttpRequest();
    virtual         ~SHttpRequest();

public:
    //! Gets the method
    const SString &Method() const;

    //! Sets the method
    void SetMethod(const SString &method);

    //! Gets the method Resource
    const SString &Resource() const;

    //! Sets the request Resource
    void SetResource(const SString &resource);

    //! Sets the request status
    int StatusCode() const;

    //! Sets the content body
    void SetContentBody(SBodyPart *pPart) { pContentBody = pPart; }

    //! Return the response for the request
    virtual SHttpResponse *Response() const { return pResponse; }

    //! Gets the content body
    SBodyPart *ContentBody() const { return pContentBody; }

    //! The status message
    const SString StatusMessage() const;

    //! Sets the request status
    void SetStatus(int status, const SString &msg);

    //! Parse the first line
    bool ParseFirstLine(const SString &line);

protected:
    //! Reads the first request line
    virtual bool ReadFirstLine(std::istream &input);

protected:
    //! Current request method
    SString     method;

    //! Resource being accessed
    SString     resource;

    //! The actually data that is sent as the content - Usually POSTs
    SBodyPart *     pContentBody;

    //! The response for this request
    SHttpResponse * pResponse;
};

#endif
