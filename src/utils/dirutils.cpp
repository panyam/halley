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
 *  \file   dirutils.cpp
 *
 *  \brief  Utilities for Directory reading.
 *
 *  \version
 *      - Sri Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "dirutils.h"

typedef int(*VoidPtrCompareFunc)(const void *a, const void *b);

int DirEnt::filterDotAndDotDot(const struct dirent *pDirEnt)
{
    // ignore "." and ".." entries
    if ((pDirEnt->d_name[0] == '.' && pDirEnt->d_name[1] == 0) ||
        (pDirEnt->d_name[0] == '.' && (pDirEnt->d_name[1] == '.' && pDirEnt->d_name[2] == 0)))
        return 0;
    return 1;
}

//! Reads a directory and stores contents in the entries list
// Returns false if directory could not be read.
bool DirEnt::ReadDirectory(const char *dirname,
                           std::list<DirEnt> &entries,
                           int(*compareFunc)(const struct dirent **, const struct dirent **),
                           int(*filterFunc)(const struct dirent *))
{
    if (compareFunc == NULL && (filterFunc == NULL || filterFunc == filterDotAndDotDot))
    {
        // better version with JUST reading and no sorting
        DIR *pDir = opendir(dirname);
        if (pDir == NULL)
            return false;

        struct dirent *pDirEnt = readdir(pDir);
        while (pDirEnt != NULL)
        {
            // ignore "." and ".." entries
            if (filterDotAndDotDot(pDirEnt) != 0)
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
    }
    else
    {
        struct dirent **namelist;

        int numFiles = scandir(dirname, &namelist, filterFunc, (VoidPtrCompareFunc)compareFunc);
        if (numFiles >= 0)
        {
            for (int i = 0;i < numFiles;i++)
            {
                struct dirent *pDirEnt = namelist[i];
                // ignore "." and ".." entries
                if (filterDotAndDotDot(pDirEnt) != 0)
                {
                    DirEnt entry(pDirEnt->d_name);
                    std::stringstream entnamestream;
                    entnamestream << dirname << "/" << entry.entName;
                    stat(entnamestream.str().c_str(), &entry.entStat);
                    entries.push_back(entry);
                }
                free(namelist[i]);
            }
            free(namelist);
        }
    }

    return true;
}

namespace FileUtils
{

//*****************************************************************************
/*!
 *  \brief  Returns a proper error message string for an errno value.
 *
 *  \version
 *      - Sri Panyam      18/03/2010
 *        Created.
 *
 *****************************************************************************/
std::string FileErrorNumToString(int errnum)
{
    std::string errormsg;
    switch (errnum)
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
    return errormsg;
}

//*****************************************************************************
/*!
 *  \brief  Tries to open a file and return its file pointer.
 *
 *  \version
 *      - Sri Panyam      10/03/2009
 *        Created.
 *
 *****************************************************************************/
FILE *OpenFile(const char *filename, const char *mode, std::string &errormsg)
{
    FILE *fptr  = fopen(filename, mode);
    if (fptr == NULL)
    {
        errormsg = FileErrorNumToString(errno);
    }

    return fptr;
}

//*****************************************************************************
/*!
 *  \brief  Tries to open a file and returns the file descriptor.
 *
 *  \version
 *      - Sri Panyam      18/03/2010
 *        Created.
 *
 *****************************************************************************/
int OpenFD(const char *filename, int flags, std::string &errormsg)
{
    int fd = open(filename, flags);
    if (fd < 0)
    {
        errormsg = FileErrorNumToString(errno);
    }
    return fd;
}

}
