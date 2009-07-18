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
 *  \file   filemodule.cpp
 *
 *  \brief  Module for routing to other modules based on Urls.
 *
 *  \version
 *      - Sri Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include <dirent.h>
#include "filemodule.h"
#include "handlerstage.h"
#include "request.h"
#include "response.h"
#include "mimetypes.h"

//! Called to handle input data from another module
// This module simply writes out a given file and calls "ProcessOutput of
// the next module.
void SFileModule::ProcessInput(SConnection *        pConnection,
                               SHttpHandlerData *   pHandlerData,
                               SHttpHandlerStage *  pStage,
                               SBodyPart *          pBodyPart)
{
    SHttpRequest *pRequest  = pHandlerData->Request();
    SString resource        = pRequest->Resource();

    SString docroot;
    SString filename;
    SString prefix;

    SString     errormsg;
    SBodyPart *     part        = NULL;
    SHttpResponse * pResponse   = pRequest->Response();
    SHeaderTable &  respHeaders(pResponse->Headers());

    // evaluate filename from resource
    if (!ParsePath(resource, docroot, filename, prefix))
    {
        pResponse->SetStatus(404, "Not Found");
        respHeaders.SetIntHeader("Content-Length", 0);
        respHeaders.SetHeader("Content-Type", "text/text");
    }
    else
    {
        SString fullpath = docroot + filename;
        struct stat fileStat;
        memset(&fileStat, 0, sizeof(struct stat));
        if (stat(fullpath.c_str(), &fileStat) != 0)
        {
            int statcode    = 500;
            int errnum      = errno;
            SLogger::Get()->Log("ERROR: Could not stat file: %s, Error [%d]: %s\n",
                                fullpath.c_str(), errno, strerror(errno));
            switch (errnum)
            {
                case EFAULT:
                    errormsg = "Bad address.";
                    break;
                case EACCES:
                    errormsg = "Search permission is denied for one of the directories in the path prefix of path.  (See also path_resolution(2).)";
                    break;
                case EBADF:
                    errormsg = "filedes is bad.";
                    break;
                case ELOOP:
                    errormsg = "Too many symbolic links encountered while traversing the path.";
                    break;
                case ENAMETOOLONG:
                    errormsg = "File name too long.";
                    break;
                case ENOENT:
                    errormsg = "A component of the path path does not exist, "
                               "or the path is an empty string.";
                    break;
                case ENOMEM:
                    errormsg = "Out of memory (i.e. kernel memory)." ;
                    break;
                case ENOTDIR:
                    errormsg = "A component of the path is not a directory.";
                    break;
                default:
                    errormsg = "Unknown Error.";
                    break;
            }

            pResponse->SetStatus(statcode, "Cannot read file");
            respHeaders.SetIntHeader("Content-Length", errormsg.size());
            respHeaders.SetHeader("Content-Type", "text/text");
            SRawBodyPart *pRawPart = pResponse->NewRawBodyPart();
            pRawPart->SetBody(errormsg);
            part = pRawPart;
        }
        else
        {
            if ((fileStat.st_mode & S_IFDIR) != 0)
            {
                if (showIndexes)
                {
                }

                // we are dealing with a folder!
                SString format      = pRequest->GetQueryValue("format");
                bool raw            = (strcasecmp(format.c_str(), "raw") == 0);
                SString contents    = PrintDirContents(docroot, filename, prefix, raw);
                respHeaders.SetIntHeader("Content-Length", contents.size());
                respHeaders.SetHeader("Content-Type", (raw ? "text/text" : "text/html"));
                respHeaders.SetHeader("Cache-Control", "no-cache");

                SRawBodyPart *pRawPart = pResponse->NewRawBodyPart();
                pRawPart->SetBody(contents);
                part = pRawPart;
            }
            else
            {
                // SendFile(fullpath, fileStat, part, pResponse, respHeaders);
                // respHeaders.SetIntHeader("Content-Length", fileStat.st_size);
                // respHeaders.SetHeader("Content-Type", SMimeTypes::GetInstance()->GetMimeType(fullpath));
                part = pResponse->NewFileBodyPart(fullpath);
            }
        }
    }

    pStage->SendEvent_OutputToModule(pConnection, pNextModule, part);
    pStage->SendEvent_OutputToModule(pConnection, pNextModule,
                           pResponse->NewContFinishedPart(pNextModule));
}

//*****************************************************************************
/*!
 *  \brief  Helper to send down a file.
 *
 *  \version
 *      - Sri Panyam      16/04/2009
 *        Created.
 *
 *****************************************************************************/
void SFileModule::SendFile(const SString &  fullpath,
                           struct stat      fileStat,
                           SRawBodyPart *   pPart,
                           SHttpResponse *  pResponse,
                           SHeaderTable &   respHeaders)
{
    SString errormsg;
    FILE *fptr = OpenFile(fullpath.c_str(), "rb", errormsg);
    if (fptr == NULL)
    {
        pResponse->SetStatus(404, "Cannot read file");
        respHeaders.SetIntHeader("Content-Length", errormsg.size());
        respHeaders.SetHeader("Content-Type", "text/text");

        pPart->SetBody(errormsg);
    }
    else
    {
        respHeaders.SetIntHeader("Content-Length", fileStat.st_size);
        respHeaders.SetHeader("Content-Type", SMimeTypes::GetInstance()->GetMimeType(fullpath));

        const static int MAX_READ_SIZE = (1 << 15);
        char fileBuffer[MAX_READ_SIZE];
        int nRead = 0;
        while ((nRead = fread(fileBuffer, 1, MAX_READ_SIZE, fptr)) > 0)
        {
            pPart->AppendToBody(fileBuffer, nRead);
        }
        fclose(fptr);
    }
}

//*****************************************************************************
/*!
 *  \brief  Goes through our docroot table and sees which doc root the
 *  given path matches.
 *
 *  \version
 *      - Sri Panyam      11/03/2009
 *        Created.
 *
 *****************************************************************************/
bool SFileModule::ParsePath(const SString &path, SString &docroot, SString &child, SString &prefix)
{
    for (std::list<SStringPair>::iterator iter = docRoots.begin();
                iter != docRoots.end(); ++iter)
    {
        SStringPair item    = *iter;
        size_t      preflen = iter->first.size();
        if (strncmp(path.c_str(), iter->first.c_str(), preflen) == 0)
        {
            prefix = iter->first;
            docroot = iter->second;
            child = SString(path.c_str() + preflen);
            return true;
        }
    }
    return false;
}

//*****************************************************************************
/*!
 *  \brief  Tries to open a file.
 *
 *  \version
 *      - Sri Panyam      10/03/2009
 *        Created.
 *
 *****************************************************************************/
FILE * SFileModule::OpenFile(const char *filename, const char *mode, SString &errormsg)
{
    errormsg    = "Unknown Error";

    FILE *fptr  = fopen(filename, mode);
    if (fptr == NULL)
    {
        switch (errno)
        {
        case EEXIST:
            errormsg = "pathname already exists and O_CREAT and O_EXCL were used.";
            break;
        case EISDIR:
            errormsg = "pathname refers to a directory and the access "
                       "requested involved writing (that is, O_WRONLY "
                       "or O_RDWR is set).";
            break ;
        case EACCES:
            errormsg = "The requested access to the file is not allowed, or "
                       "one  of the directories  in  pathname did not allow "
                       "search (execute) permission, or the file did not "
                       "exist yet and write access to the parent directory "
                       "is not allowed.";
            break ;
        case ENAMETOOLONG:
            errormsg = "pathname was too long.";
            break ;
        case ENOENT:
            errormsg = "O_CREAT  is  not  set  and the named file does not "
                       "exist.  Or, a directory component in pathname does "
                       "not exist or is a dangling symbolic link.";
            break ;
        case ENOTDIR:
            errormsg = "A component  used as a directory in pathname is not, "
                       "in fact, a directory, or O_DIRECTORY was specified "
                       "and pathname was not a directory.";
            break ;
        case ENXIO:
            errormsg = "O_NONBLOCK  |  O_WRONLY  is set, the named file is a "
                       "FIFO and no process has the file open for reading.  "
                       "Or, the file is a device special file and no "
                       "corresponding device exists.";
            break ;
        case ENODEV:
            errormsg = "pathname  refers  to  a device special file and no "
                       "corresponding device exists.  (This is a Linux kernel "
                       "bug - in this situation ENXIO must be returned.)";
            break ;
        case EROFS:
            errormsg = "pathname  refers  to  a file on a read-only "
                       "filesystem and write access was requested.";
            break ;
        case ETXTBSY:
            errormsg =  "pathname refers to an executable image which is "
                        "currently being executed and write access was requested.";
            break ;
        case EFAULT:
            errormsg =  "pathname points outside your accessible address space.";
            break;
        case ELOOP:
            errormsg =  "Too many symbolic links were encountered in resolving "
                        "pathname, or O_NOFOLLOW was specified but pathname "
                        "was a symbolic link.";
            break ;
        case ENOSPC:
            errormsg =  "pathname was to be created but the  device  containing "
                        "pathname has no room for the new file.";
            break ;
        case ENOMEM:
            errormsg =  "Insufficient kernel memory was available.";
            break ;
        case EMFILE:
            errormsg =  "The process already has the maximum number of files open.";
            break ;
        case ENFILE:
            errormsg =  "The  limit  on  the total number of files open on the system "
                        "has been reached.";
            break ;
        }
    }

    return fptr;
}

//! Reads a directory and stores contents in the entries vector.
// Returns false if directory could not be read.
bool SFileModule::ReadDirectory(const char *dirname, std::vector<DirEnt> &entries)
{
    // read directory contents
    DIR *pDir = opendir(dirname);
    if (pDir == NULL)
        return false;

    struct dirent *pDirEnt = readdir(pDir);
    while (pDirEnt != NULL)
    {
        // ignore "." and ".." entries
        if (!(pDirEnt->d_name[0] == '.' && pDirEnt->d_name[1] == 0) &&
            !(pDirEnt->d_name[0] == '.' && (pDirEnt->d_name[1] == '.' || pDirEnt->d_name[2] == 0)))
        {
            DirEnt entry(pDirEnt->d_name);
            SStringStream entnamestream;
            entnamestream << dirname << "/" << entry.entName;
            stat(entnamestream.str().c_str(), &entry.entStat);
            entries.push_back(entry);
        }
        pDirEnt = readdir(pDir);
    }
    closedir(pDir);

    return true;
}

//*****************************************************************************
/*!
 *  \brief  Returns directory contents as a html formatted string
 *
 *  \param  const char *dirname     Name of the directory to list.
 *
 *  \return The html formatted directory contents string
 *
 *  \version
 *      - Sri Panyam      02/02/2009
 *        Created.
 *
 *****************************************************************************/
SString SFileModule::PrintDirContents(const SString &docroot, const SString &filename, const SString &prefix, bool raw)
{
    SString dirname(docroot + filename);
    int filenamelen = filename.size();
    SStringStream output;
    std::vector<DirEnt> entries;

    if (raw)
    {
        output << "[";
    }
    else
    {
        output << "<html>";
        output << "<head>";
        output << "</head>";
        output << "<body>";
        output << "<p><center><h2>Contents of: ";

        // show all parent directories for easy access
        output << PrintDirParents(prefix, filename);

        output << "</h2></center>";
        output << "<hl></hl>";
    }

    if (ReadDirectory(dirname.c_str(), entries))
    {
        // sort it
        std::sort(entries.begin(), entries.end(), DirEnt::DirEntCmp);
        std::vector<DirEnt>::iterator iter = entries.begin();

        if (!raw)
        {
            output << "<table width = \"100%\">";
            output << "<thead>";
            output << "<tr>";
            output << "<td><strong>File Name</strong></td>";
            output << "<td><strong>Size</strong></td>";
            // output << "<td><strong>Created</strong></td>";
            // output << "<td><strong>Modified</strong></td>";
            output << "</tr>";
            output << "</thead>";
        }
        for (;iter != entries.end(); ++iter)
        {
            bool isdir = (iter->entStat.st_mode & S_IFDIR) != 0;

            if (raw)
            {
                output << "{'name': '" << iter->entName << "', " << 
                            "'isdir': " << isdir << ", " <<
                            "'size': " << iter->entStat.st_size << "}" << std::endl;
            }
            else
            {
                output << "<tr>";
                output << "<td><a href=\"";
                output << prefix;
                output << (filename[0] == '/' ? filename.c_str() + 1 : filename.c_str());
                if (filenamelen > 0 && filename[filenamelen - 1] != '/') output << "/";
                output << (iter->entName[0] == '/' ? iter->entName.c_str() + 1 : iter->entName.c_str());
                output << "\">";

                if (isdir)
                {
                    output << "[" << iter->entName << "]</a></td>";
                    output << "<td>---</td>";
                }
                else
                {
                    output << iter->entName << "</a></td>";
                    output << "<td>" << iter->entStat.st_size << "</td>";
                }
                output << "</tr>";
            }
        }
        if (!raw)
        {
            output << "</table>";
        }
    }
    else
    {
        int errnum = errno;
        output << "Error: Cannot open directory: " << strerror(errnum);
    }

    if (raw)
    {
        output << "]" << std::endl;
    }
    else
    {
        output << "<hl></hl>";
        output << "</body>";
        output << "</html>";
    }
    return output.str();
}

//*****************************************************************************
/*!
 *  \brief  Returns a path as a list of parent folder names formatted as
 *  html
 *
 *  \param  const char *dirname     Name of the directory to list.
 *
 *  \return The html formatted parent folders.
 *
 *  \version
 *      - Sri Panyam      10/03/2009
 *        Created.
 *
 *****************************************************************************/
SString SFileModule::PrintDirParents(const SString &prefix, const SString &filename)
{
    SStringStream parentPathHtml;
    SString lastPath("/");
    SString fullpath("/" + prefix + filename);
    const char *pStart      = fullpath.c_str();
    const char *pEnd        = pStart + fullpath.size();

    // skip initial slashes and spaces
    while (*pStart != 0 && (isspace(*pStart) || (*pStart == '/'))) pStart++;
    const char *pCurr       = pStart;

    parentPathHtml << "<a href='/'>[Home]</a> / " << std::endl;

    do
    {
        const char *pSlashPos   = strchr(pCurr, '/');
        if (pSlashPos == NULL) pSlashPos = pEnd;
        if (pSlashPos != pCurr + 1)
        {
            SString currdir(pCurr, pSlashPos - pCurr);
            parentPathHtml << "<a href='" << lastPath << currdir << "/'>" << currdir << "</a> / " << std::endl;
            lastPath = lastPath + currdir + "/";
        }

        pCurr = pSlashPos + 1;
    } while (pCurr < pEnd);

    if (pCurr == pStart)
        return fullpath;
    else
        return parentPathHtml.str();
}
