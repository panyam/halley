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
bool SRegexUrlMatcher::Matches(const std::string &resource) const
{
    return false;
}

//! Tells if a pattern is at the start, middle or end of a resource
bool SContainsUrlMatcher::Matches(const std::string &resource) const
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
    for (UrlMatcherList::const_iterator iter = urlMatchers.begin();
                iter != urlMatchers.end();++iter)
    {
        if ((*iter)->Matches(pRequest->Resource()))
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

