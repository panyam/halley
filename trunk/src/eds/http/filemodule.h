//*****************************************************************************
/*!
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

//! Simple structure info about a directory.
class DirEnt
{
public:
    SString entName;
    struct stat entStat;

public:
    //! Constructor
    DirEnt(const SString &name) : entName(name)
    {
        memset(&entStat, 0, sizeof(entStat));
    }

    //! Compares two directory entries
    static bool DirEntCmp(const DirEnt &lhs, const DirEnt &rhs)
    {
        if (lhs.entStat.st_mode == rhs.entStat.st_mode)
        {
            /*
            const char *pLhs = lhs.entName.c_str();
            const char *pRhs = rhs.entName.c_str();
            if (pLhs == NULL)
                return true;
            else if (pRhs == NULL)
                return false;
            else
                return strcmp(pLhs, pRhs) < 0;
            */

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
};

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
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart);

    //! Tries to open a file.
    static FILE *OpenFile(const char *filename, const char *mode, SString &errormsg);

    //! Print contents of directory
    static SString PrintDirContents(const SString &docroot, const SString &filename, const SString &prefix);

    //! Print directory parents.
    static SString PrintDirParents(const SString &docroot, const SString &filename);

    //! Reads directory contents.
    static bool ReadDirectory(const char *dirname, std::vector<DirEnt> &entries);

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

