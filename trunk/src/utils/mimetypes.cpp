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
 *  \file   mimetypes.cpp
 *
 *  \brief  Manages mime types.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/


#include <errno.h>
#include "mimetypes.h"
#include "logger/logger.h"

//! Get singleton instance.
SMimeTypes *SMimeTypes::GetInstance(const char *initial_mimeconf)
{
    static SMimeTypes theMimeTypes(initial_mimeconf);

    return &theMimeTypes;
}

//! Create a server and initialise mime tables
SMimeTypes::SMimeTypes(const char *mimetypes)
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

    FILE *mimefile = fopen(mimetypes, "r");
    if (mimefile == NULL)
    {
        SLogger::Get()->Log("ERROR: Could not fopen file: %s, Error [%d]: %s\n",
                            mimetypes, errno, strerror(errno));
        return ;
    }

    // read entries from mime.types
    char line[1025];
    while (fgets(line, 1024, mimefile) != NULL)
    {
        const char *pStart = line;
        while (*pStart != 0 && isspace(*pStart))
            pStart++;

        if (*pStart != '\n' && *pStart != 0 && *pStart != '#')
        {
            char *pSavePtr  = NULL;
            char *ptr       = strtok_r(line, " \t", &pSavePtr);

            std::string mimetype(ptr);
            while ((ptr = strtok_r(NULL, " \t", &pSavePtr)) != NULL)
            {
                mimeTypes.insert(std::pair<std::string, std::string>(ptr, mimetype));
            }
        }
    }
    fclose(mimefile);
}

// Removes mime types
SMimeTypes::~SMimeTypes()
{
    mimeTypes.clear();
}

//*****************************************************************************
/*!
 *  \brief  Sets the mime type for an extension, replacing one if it
 *  already exists.
 *
 *  \version
 *      - Sri Panyam      16/07/2009
 *        Created.
 *
 *****************************************************************************/
void SMimeTypes::SetMimeType(const std::string &ext, const std::string &mtype)
{
    mimeTypes[ext] = mtype;
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

