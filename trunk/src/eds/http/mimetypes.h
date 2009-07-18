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
 *  \file   mimetypes.h
 *
 *  \brief  Manages mime types.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef MIME_TYPE_H
#define MIME_TYPE_H

#include "httpfwd.h"

//*****************************************************************************
/*!
 *  \class  SMimeTypes
 *
 *  \brief  Simpleton class for managing mime types.
 *
 *****************************************************************************/
class SMimeTypes
{
public:
    SMimeTypes(const char *mimeconf = "/etc/mime.types");
    ~SMimeTypes();

    SString GetMimeType(const SString &filename);

    void SetMimeType(const SString &ext, const SString &mtype);

    //! Get singleton instance.
    static SMimeTypes *GetInstance();

protected:
    typedef std::map<SString, SString> MimeTable;
    MimeTable    mimeTypes;
};

#endif

