//*****************************************************************************
/*!
 *  \file   urlrouter.cpp
 *
 *  \brief  Module for routing to other modules based on Urls.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include "urlrouter.h"
#include "handlerstage.h"
#include "request.h"

//! TODO: Matches a resource to a regex.
bool SRegexUrlMatcher::Matches(const std::string &resource, SStringList &params) const
{
    return false;
}

//! Tells if a pattern is at the start, middle or end of a resource
bool SContainsUrlMatcher::Matches(const std::string &resource, SStringList &params) const
{
    const char *res_ptr = resource.c_str();
    const char *pat_ptr = resPattern.c_str();
    int         patlen  = resPattern.size();
    int         reslen  = resource.size();

    if (reslen < patlen)
        return false;

    const char* match = strstr(res_ptr, pat_ptr);
    if (match == NULL)
        return false;

    return (match == res_ptr && ((locFlags | PREFIX_MATCH) == 0)) ||
           (match == (res_ptr - patlen) && ((locFlags | SUFFIX_MATCH) == 0)) ||
           ((locFlags | MIDDLE_MATCH) == 0);
}

//! Called to handle input data from another module
void SUrlRouter::ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart)
{
    SHttpRequest *pRequest = pHandlerData->Request();
    SStringList params;
    for (UrlMatcherList::const_iterator iter = urlMatchers.begin();
                iter != urlMatchers.end();++iter)
    {
        params.clear();
        if ((*iter)->Matches(pRequest->Resource(), params))
        {
            pStage->InputToModule(pHandlerData->pConnection, (*iter)->Module());
            return ;
        }
    }

    // see if there is a default module to send to
    if (pNextModule != NULL)
    {
        pStage->InputToModule(pHandlerData->pConnection, pNextModule);
    }
    else
    {
        // TODO: return a 500 or 404 error
    }
}

