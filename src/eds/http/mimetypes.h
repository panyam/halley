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

#include <map>
#include <string>

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

    std::string GetMimeType(const std::string &filename);

    //! Get singleton instance.
    static SMimeTypes *GetInstance();

protected:
    typedef std::map<std::string, std::string> MimeTable;
    MimeTable    mimeTypes;
};

#endif

