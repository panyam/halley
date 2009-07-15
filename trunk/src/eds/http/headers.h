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
 *  \file   headers.h
 *
 *  \brief
 *  A HTTP message headers.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_HEADERS_H_
#define _SHTTP_HEADERS_H_

#include "utils.h"
#include "httpfwd.h"

typedef std::map<SString, SString>   HeaderMap;
typedef std::pair<SString, SString>  HeaderPair;
typedef std::map<SString, SString>   CookieMap;
typedef std::pair<SString, SString>  CookiePair;

//! A header in the message
class SHeader
{
public:
    //! Creates a new header
    SHeader(const SString &v = "") : value(v) { }

    //! Gets the value of the header
    const SString &Value() { return value; }

    //! Sets the value
    void SetValue(const SString &v = "")
    {
        value = v;
    }

    void AppendValue(const SString &v)
    {
        value += v;
    }

protected:
    //! Type of header
    char        type;

    //! Value of the header
    SString value;
};

//! Table of headers
class SHeaderTable
{
public:
    //! Creates a header table
    SHeaderTable() : locked(false) { }

    //! Destructor
    virtual ~SHeaderTable() { }

    //! Resets to start all over again
    virtual void Reset();

    //! Gets a header
    SString Header(const SString &name) const;

    //! Returns a header if it exists
    bool HeaderIfExists(const SString &name, SString &value);

    //! Tells if a header is available
    bool HasHeader(const SString &name) const;

    //! Sets the value of an string typed header
    void SetHeader(const SString &name, const SString &value, bool append = false);

    //! Sets the value of an bool typed header
    void SetBoolHeader(const SString &name, bool value);

    //! Sets the value of an int typed header
    void SetIntHeader(const SString &name, int value);

    //! Sets the value of an int typed header
    void SetUIntHeader(const SString &name, unsigned value);

    //! Sets the value of an double typed header
    void SetDoubleHeader(const SString &name, double value);

    //! Removes a particular header
    SString RemoveHeader(const SString &name);

    //! Parses a header line.
    virtual bool ParseHeaderLine(const SString &line, SString &name, SString &value);

    HeaderMap::const_iterator FirstHeader() { return headers.begin(); }
    HeaderMap::const_iterator LastHeader() { return headers.end(); }

    //! Reads the next header
    virtual bool ReadHeaders(std::istream &input);

    //! Reads the next header
    virtual bool ReadNextHeader(std::istream &input, SString &name, SString &value);

    //! Writes the headers to the stream
    virtual int WriteHeaders(std::ostream &output);

    //! Lock the header and prevent further changes
    inline void Lock() { locked = true; }

    //! Tells if the headers are locked
    inline bool Locked() { return locked; }

    //! Tells if the connection is to be closed or not
    inline bool CloseConnection() const { return closeConnection; }

protected:
    //! headers
    HeaderMap   headers;

    //! Quick access to whether Connection is to be closed or not
    bool        closeConnection;

    //! Tells if the headers are locked - once locked they cant be changed
    bool        locked;
};

//! 
// Accessor allows one to treat headers in a clearner way and allows
// caching of the values.
//
class SHeaderAccessor
{
public:
    //! Creates the accessor
    SHeaderAccessor(SHeader *pHdr) : changed(false), pHeader(pHdr) { }

    //! Gets the value of the header
    template <typename T> const T &Value();

protected:
    //! Has the header value changed
    bool        changed;

    //! 0 = Bool, 1 = Int, 2 = Number, 3 = String
    int         valueType;
    bool        boolValue;
    int         intValue;
    double      dblValue;
    SString     strValue;

    //! Header we are caching/proxying.
    SHeader *   pHeader;
};

#endif

