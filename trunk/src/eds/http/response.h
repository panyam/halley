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

    //! Resets to start all over again
    virtual void Reset();

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

