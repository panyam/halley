//*****************************************************************************
/*!
 *  \file   mimetypes.cpp
 *
 *  \brief  Manages mime types.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/


#include "mimetypes.h"

//! Get singleton instance.
SMimeTypes *SMimeTypes::GetInstance()
{
    static SMimeTypes theMimeTypes;

    return &theMimeTypes;
}

//! Create a server and initialise mime tables
SMimeTypes::SMimeTypes()
{
    // TODO: Need to this in a proper way that would query the system
    // (and loading the mime.conf file to memory) instead of
    // checking for filetypes manually!
    mimeTypes.insert(std::pair<std::string, std::string>("xml", "text/xml"));
    mimeTypes.insert(std::pair<std::string, std::string>("css", "text/css"));
    mimeTypes.insert(std::pair<std::string, std::string>("js", "application/x-javascript"));
    mimeTypes.insert(std::pair<std::string, std::string>("txt", "text/plain"));
    mimeTypes.insert(std::pair<std::string, std::string>("cfg", "text/text"));
    mimeTypes.insert(std::pair<std::string, std::string>("png", "image/png"));
    mimeTypes.insert(std::pair<std::string, std::string>("gif", "image/gif"));
    mimeTypes.insert(std::pair<std::string, std::string>("jpg", "image/jpeg"));
    mimeTypes.insert(std::pair<std::string, std::string>("pdf", "application/pdf"));
    mimeTypes.insert(std::pair<std::string, std::string>("doc", "application/msword"));
    mimeTypes.insert(std::pair<std::string, std::string>("bin", "application/octet-stream"));
    mimeTypes.insert(std::pair<std::string, std::string>("swf", "application/x-shockwave-flash"));
    mimeTypes.insert(std::pair<std::string, std::string>("zip", "application/zip"));
    mimeTypes.insert(std::pair<std::string, std::string>("bz2", "application/x-bzip2"));
    mimeTypes.insert(std::pair<std::string, std::string>("html", "text/html"));
    mimeTypes.insert(std::pair<std::string, std::string>("htm", "text/html"));
    mimeTypes.insert(std::pair<std::string, std::string>("wav", "audio/x-wav"));
}

// Removes mime types
SMimeTypes::~SMimeTypes()
{
    mimeTypes.clear();
}

//*****************************************************************************
/*!
 *  \brief  Return's a file's mime type.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
std::string SMimeTypes::GetMimeType(const std::string &filename)
{
    const char *pStart = filename.c_str();
    const char *pCurr = pStart + filename.size() - 1;

    while (pCurr > pStart && *pCurr != '.')
        pCurr --;

    if (pCurr > pStart)
    {
        pCurr++;

        std::map<std::string, std::string>::iterator iter = mimeTypes.find(pCurr);

        if (iter != mimeTypes.end())
        {
            return iter->second;
        }
    }

    return "application/binary";
}

