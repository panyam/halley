//*****************************************************************************
/*!
 *  \file   filemodule.cpp
 *
 *  \brief  Module for routing to other modules based on Urls.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include <dirent.h>
#include "filemodule.h"
#include "handlerstage.h"
#include "request.h"
#include "response.h"

//! Called to handle input data from another module
// This module simply writes out a given file and calls "ProcessOutput of
// the next module.
void SFileModule::ProcessInput(SHttpHandlerData *   pHandlerData,
                               SHttpHandlerStage *  pStage,
                               SBodyPart *          pBodyPart)
{
    SHttpRequest *pRequest  = pHandlerData->Request();
    std::string resource    = pRequest->Resource();

    std::string docroot;
    std::string filename;
    std::string prefix;

    std::string     errormsg;
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
        std::string fullpath = docroot + filename;
        part = pResponse->NewBodyPart();
        struct stat fileStat;
        memset(&fileStat, 0, sizeof(struct stat));
        if (stat(fullpath.c_str(), &fileStat) != 0)
        {
            int statcode = 500;
            switch (errno)
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
                    errormsg = "A component of the path path does not exist, or the path is an empty string.";
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
            part->SetBody(errormsg);
        }
        else
        {
            if ((fileStat.st_mode & S_IFDIR) != 0)
            {
                // we are dealing with a folder!
                std::string contents = PrintDirContents(docroot, filename, prefix);
                respHeaders.SetIntHeader("Content-Length", contents.size());
                respHeaders.SetHeader("Content-Type", "text/html");
                respHeaders.SetHeader("Cache-Control", "no-cache");

                part->SetBody(contents);
            }
            else
            {
                FILE *fptr = OpenFile(fullpath.c_str(), "rb", errormsg);
                if (fptr == NULL)
                {
                    pResponse->SetStatus(404, "Cannot read file");
                    respHeaders.SetIntHeader("Content-Length", errormsg.size());
                    respHeaders.SetHeader("Content-Type", "text/text");

                    part->SetBody(errormsg);
                }
                else
                {
                    respHeaders.SetIntHeader("Content-Length", fileStat.st_size);
                    respHeaders.SetHeader("Content-Type", "text/text");

                    const static int MAX_READ_SIZE = (1 << 15);
                    char fileBuffer[MAX_READ_SIZE];
                    int nRead = 0;
                    while ((nRead = fread(fileBuffer, 1, MAX_READ_SIZE, fptr)) > 0)
                    {
                        part->AppendToBody(fileBuffer, nRead);
                    }
                    fclose(fptr);
                }
            }
        }
    }
    pStage->OutputToModule(pHandlerData->pConnection, pNextModule, part);
    pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                           pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED, pNextModule));
}

//*****************************************************************************
/*!
 *  \brief  Goes through our docroot table and sees which doc root the
 *  given path matches.
 *
 *  \version
 *      - S Panyam      11/03/2009
 *        Created.
 *
 *****************************************************************************/
bool SFileModule::ParsePath(const std::string &path, std::string &docroot, std::string &child, std::string &prefix)
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
            child = std::string(path.c_str() + preflen);
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
 *      - S Panyam      10/03/2009
 *        Created.
 *
 *****************************************************************************/
FILE * SFileModule::OpenFile(const char *filename, const char *mode, std::string &errormsg)
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
            !(pDirEnt->d_name[0] == '.' && pDirEnt->d_name[1] == '.' ||
                    pDirEnt->d_name[2] == 0))
        {
            DirEnt entry(pDirEnt->d_name);
            std::stringstream entnamestream;
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
 *      - S Panyam      02/02/2009
 *        Created.
 *
 *****************************************************************************/
std::string SFileModule::PrintDirContents(const std::string &docroot, const std::string &filename, const std::string &prefix)
{
    std::string dirname(docroot + filename);
    int dirnamelen = dirname.size();
    std::stringstream output;
    std::vector<DirEnt> entries;

    output << "<html>";
    output << "<head>";
    output << "</head>";
    output << "<body>";
    output << "<p><center><h2>Contents of: ";

    // show all parent directories for easy access
    output << PrintDirParents(docroot, filename);

    output << "</h2></center>";
    output << "<hl></hl>";

    if (ReadDirectory(dirname.c_str(), entries))
    {
        // sort it
        std::sort(entries.begin(), entries.end(), DirEnt::DirEntCmp);
        std::vector<DirEnt>::iterator iter = entries.begin();

        output << "<table width = \"100%\">";
        output << "<thead>";
        output << "<tr>";
        output << "<td><strong>File Name</strong></td>";
        output << "<td><strong>Size</strong></td>";
        // output << "<td><strong>Created</strong></td>";
        // output << "<td><strong>Modified</strong></td>";
        output << "</tr>";
        output << "</thead>";
        for (;iter != entries.end(); ++iter)
        {
            bool isdir = (iter->entStat.st_mode & S_IFDIR) != 0;
            output << "<tr>";

            output << "<td><a href=\"";
            output << prefix;
            output << (dirname[0] == '/' ? dirname.c_str() + 1 : dirname.c_str());
            // output << dirname;
            if (dirnamelen > 0 && dirname[dirnamelen - 1] != '/') output << "/";
            output << (iter->entName[0] == '/' ? iter->entName.c_str() + 1 : iter->entName.c_str());
            output << "\">";

            // if (dirname[0] != '/') output << "/";

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
        output << "</table>";
    }
    else
    {
        output << "Error: Cannot open directory: " << strerror(errno);
    }

    output << "<hl></hl>";
    output << "</body>";
    output << "</html>";
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
 *      - S Panyam      10/03/2009
 *        Created.
 *
 *****************************************************************************/
std::string SFileModule::PrintDirParents(const std::string &docroot, const std::string &filename)
{
    /*
    char *buff = strdup(dirname.c_str());

    std::string bn = basename(buff);
    std::string dn = dirname(buff);

    std::string temp;

    while (! (dn[0] == 0 || ((dn[0] == '.' || dn[0] == '/') && dn[1] == 0)))
    {
        temp = 
        cout << "Bn: " << bn << ", dn: " << dn << endl;

        temp = "<a href=\"" + dn + "\">" + 
        &lt;home&gt;</a>"
        bn = basename(buff);
        dn = dirname(buff);
    }
    cout << "Bn: " << bn << ", dn: " << dn << endl;

    free(buff);

    return "<a href=\"/\">&lt;home&gt;</a>" + temp 
    */

    return docroot + filename;
}
