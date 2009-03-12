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
    const std::string &Method() const;

    //! Sets the method
    void SetMethod(const std::string &method);

    //! Gets the method Resource
    const std::string &Resource() const;

    //! Sets the request Resource
    void SetResource(const std::string &resource);

    //! Sets the request status
    int StatusCode() const;

    //! Sets the content body
    void SetContentBody(SBodyPart *pPart) { pContentBody = pPart; }

    //! Return the response for the request
    virtual SHttpResponse *Response() const { return pResponse; }

    //! Gets the content body
    SBodyPart *ContentBody() const { return pContentBody; }

    //! The status message
    const std::string StatusMessage() const;

    //! Sets the request status
    void SetStatus(int status, const std::string &msg);

    //! Parse the first line
    bool ParseFirstLine(const std::string &line);

protected:
    //! Reads the first request line
    virtual bool ReadFirstLine(std::istream &input);

protected:
    //! Current request method
    std::string     method;

    //! Resource being accessed
    std::string     resource;

    //! The actually data that is sent as the content - Usually POSTs
    SBodyPart *     pContentBody;

    //! The response for this request
    SHttpResponse * pResponse;
};

#endif

