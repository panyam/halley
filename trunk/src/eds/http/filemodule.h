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
 *  \file   filemodule.h
 *
 *  \brief  A module for serving static files.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SFILE_MODULE_H_
#define _SFILE_MODULE_H_

#include "httpmodule.h"

//! A module for serving static files relative to a doc root folder.
class SFileModule : public SHttpModule
{
public:
    //! Creates the file module
    SFileModule(SHttpModule *pNext, bool indexes = false) :
        SHttpModule(pNext), showIndexes(indexes) { }

    //! Destructor 
    virtual ~SFileModule() { }

    //! Called to handle input data from another module
    virtual void ProcessInput(SConnection *         pConnection,
                              SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart);

    //! Tries to open a file.
    static FILE *OpenFile(const char *filename, const char *mode, SString &errormsg);

    //! Helper to send file
    static void SendFile(const SString &    fullpath,
                         struct stat        fileStat,
                         SRawBodyPart *     pPart,
                         SHttpResponse *    pResponse,
                         SHeaderTable &     respHeaders);

    //! Print contents of directory
    static SString PrintDirContents(const SString &docroot, const SString &filename, const SString &prefix, bool raw = false);

    //! Print directory parents.
    static SString PrintDirParents(const SString &docroot, const SString &filename);

    //! Adds a new docroot
    virtual void AddDocRoot(const SString &prefix, const SString &docRoot)
    {
        docRoots.push_back(SStringPair(prefix, docRoot));
    }

    //! Parses a resource path to its docroot and child path components
    virtual bool ParsePath(const SString &path, SString &docroot, SString &child, SString &prefix);

protected:
    //! The document roots for each prefix
    std::list<SStringPair> docRoots;

    //! Whether (sub) directory's can be listed
    bool            showIndexes;
};

#endif

