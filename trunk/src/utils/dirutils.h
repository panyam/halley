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
 *  \file   dirutils.h
 *
 *  \brief  Few file/directory utilities
 *
 *  \version
 *      - S Panyam      18/03/2010
 *        Created
 *
 *****************************************************************************/

#ifndef __SDIR_UTILITIES_H__
#define __SDIR_UTILITIES_H__

#include <string>
#include <sstream>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


typedef int(*DirEntFilterFunc)(const struct dirent *);
typedef int(*DirEntCompareFunc)(const struct dirent **,const struct dirent **);

//! Simple structure info about a directory.
class DirEnt
{
public:
    std::string entName;
    struct stat entStat;

public:
    //! Constructor
    DirEnt(const std::string &name) : entName(name)
    {
        memset(&entStat, 0, sizeof(entStat));
    }

    //! Compares two directory entries
    static bool DirEntCmp(const DirEnt &lhs, const DirEnt &rhs)
    {
        if (lhs.entStat.st_mode == rhs.entStat.st_mode)
        {
            return std::lexicographical_compare(
                        lhs.entName.begin(), lhs.entName.end(),
                        rhs.entName.begin(), rhs.entName.end());
        }
        else
        {
            // if one is a directory it is lower in the list
            // if ((lhs.entStat.st_mode & S_IFDIR) == 0) return true;
        }
        return lhs.entStat.st_mode > rhs.entStat.st_mode;
    }

    //! Filters out "." and ".."
    static int filterDotAndDotDot(const struct dirent *);

    //! Read directory contents with optional filtering and sorting options
    static bool ReadDirectory(const char *dirname,
                              std::list<DirEnt> &entries,
                              int(*compar)(const struct dirent **, const struct dirent **) = NULL,
                              int(*filter)(const struct dirent *) = filterDotAndDotDot);
};

namespace FileUtils
{

std::string FileErrorNumToString(int errnum);
FILE *OpenFile(const char *filename, const char *mode, std::string &errormsg);
int OpenFD(const char *filename, int mode, std::string &errormsg);
size_t FileSize(const char *filename);

}

#endif
