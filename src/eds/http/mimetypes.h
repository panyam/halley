//*****************************************************************************
/*!
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
    SMimeTypes();
    ~SMimeTypes();

    SString GetMimeType(const SString &filename);

    //! Get singleton instance.
    static SMimeTypes *GetInstance();

protected:
    typedef std::map<SString, SString> MimeTable;
    MimeTable    mimeTypes;
};

#endif

