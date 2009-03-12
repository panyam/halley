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
    std::string entName;
    struct stat entStat;

public:
    //! Constructor
    DirEnt(const std::string &name) : entName(name)
    {
        memset(&entStat, 0, sizeof(entStat));
    }

    //! Compares two directory entries
    static bool DirEntCmp(const struct DirEnt &lhs, const struct DirEnt &rhs)
    {
        if (lhs.entStat.st_mode == rhs.entStat.st_mode)
        {
            return strcmp(lhs.entName.c_str(), rhs.entName.c_str()) < 0;
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
    static FILE *OpenFile(const char *filename, const char *mode, std::string &errormsg);

    //! Print contents of directory
    static std::string PrintDirContents(const std::string &docroot, const std::string &filename, const std::string &prefix);

    //! Print directory parents.
    static std::string PrintDirParents(const std::string &docroot, const std::string &filename);

    //! Reads directory contents.
    static bool ReadDirectory(const char *dirname, std::vector<DirEnt> &entries);

    //! Adds a new docroot
    virtual void AddDocRoot(const std::string &prefix, const std::string &docRoot)
    {
        docRoots.push_back(std::pair<std::string, std::string>(prefix, docRoot));
    }

    //! Parses a resource path to its docroot and child path components
    virtual bool ParsePath(const std::string &path, std::string &docroot, std::string &child, std::string &prefix);

protected:
    //! The document roots for each prefix
    std::list<std::pair<std::string, std::string> > docRoots;

    //! Whether (sub) directory's can be listed
    bool            showIndexes;
};

#endif

