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
#include "dirutils.h"

//! Reads a directory and stores contents in the entries vector.
// Returns false if directory could not be read.
bool DirEnt::ReadDirectory(const char *dirname, std::vector<DirEnt> &entries)
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

