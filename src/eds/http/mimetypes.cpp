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


#include "mimetypes.h"

//! Get singleton instance.
SMimeTypes *SMimeTypes::GetInstance()
{
    static SMimeTypes theMimeTypes;

    return &theMimeTypes;
}

//! Create a server and initialise mime tables
SMimeTypes::SMimeTypes(const char *mimetypes)
{
    // TODO: Need to this in a proper way that would query the system
    // (and loading the mime.conf file to memory) instead of
    // checking for filetypes manually!
    mimeTypes.insert(SStringPair("xml", "text/xml"));
    mimeTypes.insert(SStringPair("css", "text/css"));
    mimeTypes.insert(SStringPair("js", "application/x-javascript"));
    mimeTypes.insert(SStringPair("txt", "text/plain"));
    mimeTypes.insert(SStringPair("cfg", "text/text"));
    mimeTypes.insert(SStringPair("png", "image/png"));
    mimeTypes.insert(SStringPair("gif", "image/gif"));
    mimeTypes.insert(SStringPair("jpg", "image/jpeg"));
    mimeTypes.insert(SStringPair("pdf", "application/pdf"));
    mimeTypes.insert(SStringPair("doc", "application/msword"));
    mimeTypes.insert(SStringPair("bin", "application/octet-stream"));
    mimeTypes.insert(SStringPair("swf", "application/x-shockwave-flash"));
    mimeTypes.insert(SStringPair("zip", "application/zip"));
    mimeTypes.insert(SStringPair("bz2", "application/x-bzip2"));
    mimeTypes.insert(SStringPair("html", "text/html"));
    mimeTypes.insert(SStringPair("htm", "text/html"));
    mimeTypes.insert(SStringPair("wav", "audio/x-wav"));

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

            SString mimetype(ptr);
            while ((ptr = strtok_r(line, " \t", &pSavePtr)) != NULL)
            {
                mimeTypes.insert(SStringPair(ptr, mimetype));
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
void SMimeTypes::SetMimeType(const SString &ext, const SString &mtype)
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
SString SMimeTypes::GetMimeType(const SString &filename)
{
    const char *pStart = filename.c_str();
    const char *pCurr = pStart + filename.size() - 1;

    while (pCurr > pStart && *pCurr != '.')
        pCurr --;

    if (pCurr > pStart)
    {
        pCurr++;

        std::map<SString, SString>::iterator iter = mimeTypes.find(pCurr);

        if (iter != mimeTypes.end())
        {
            return iter->second;
        }
    }

    return "application/binary";
}

