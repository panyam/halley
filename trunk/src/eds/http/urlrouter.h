//*****************************************************************************
/*!
 *  \file   urlrouter.h
 *
 *  \brief  Module for routing to other modules based on Urls.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SURL_ROUTER_H_
#define _SURL_ROUTER_H_

#include "httpmodule.h"

//! A Url Matcher
class SUrlMatcher
{
public:
    //! Creates a new matcher
    SUrlMatcher(SHttpModule *pMatch) : pMatchModule(pMatch) { }

    //! Destructor 
    virtual ~SUrlMatcher() { }

    //! Tells if a resources matches a given pattern, and fills the params
    // array with parameters and their values.
    virtual bool Matches(const std::string &resource) const { return false; }

    //! Gets the module on pattern match
    SHttpModule *Module() const { return pMatchModule; }

public:
    // The module to be used if there is a match
    SHttpModule *pMatchModule;
};

//! Default matcher for strict matching
class SDefaultUrlMatcher : public SUrlMatcher
{
public:
    //! Creates a new matcher
    SDefaultUrlMatcher(std::string match, SHttpModule *pMatch) :
        SUrlMatcher(pMatch), absPath(match) { }

    //! Tells if a resources matches a given pattern, and fills the params
    // array with parameters and their values.
    virtual bool Matches(const std::string &resource) const
    {
        return resource == absPath;
    }

protected:
    std::string absPath;
};

class SContainsUrlMatcher : public SUrlMatcher
{
public:
    enum
    {
        PREFIX_MATCH,
        MIDDLE_MATCH,
        SUFFIX_MATCH
    };

public:
    //! Creates a new matcher
    SContainsUrlMatcher(std::string pattern, int flags, SHttpModule *pMatch) :
        SUrlMatcher(pMatch), resPattern(pattern), locFlags(flags) { }

    //! Tells if a resources matches a given pattern, and fills the params
    // array with parameters and their values.
    virtual bool Matches(const std::string &resource) const;

protected:
    //! The pattern to look for
    std::string resPattern;

    //! Tells where in the resource the pattern is to be found - PREFIX,
    // MIDDLE, or SUFFIX
    int         locFlags;
};

//! Regex matcher for strict matching
class SRegexUrlMatcher : public SUrlMatcher
{
public:
    //! Creates a new matcher
    SRegexUrlMatcher(std::string pattern, SHttpModule *pMatch) :
        SUrlMatcher(pMatch), resPattern(pattern) { }

    //! Matches the resource to a regex - TBD
    virtual bool Matches(const std::string &resource) const;

protected:
    std::string resPattern;
};

// Stores the handler state data
class SUrlRouter : public SHttpModule
{
public:
    //! Create with empty next module
    SUrlRouter(SHttpModule *pDefaultModule) : SHttpModule(pDefaultModule) { }

    //! Destroy the state
    virtual ~SUrlRouter() { }

    //! Adds a url matcher
    virtual void AddUrlMatch(const SUrlMatcher *pMatcher) { urlMatchers.push_back(pMatcher); }

public:
    typedef std::list<const SUrlMatcher *>  UrlMatcherList;
    //! List of Url Matchers to use
    UrlMatcherList      urlMatchers;

    //! Called to handle input data from another module
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart);
};

#endif

