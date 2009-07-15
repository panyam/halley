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
 *  \file   readerstage.cpp
 *
 *  \brief  The stage that asynchronously reads http requests of the socket.
 *
 *  \version
 *      - S Panyam      19/02/2009
 *        Created
 *
 *****************************************************************************/

#include "request.h"
#include "../connection.h"
#include "readerstage.h"
#include "handlerstage.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

// data and state stored by the request reader
class SHttpReaderState
{
public:
    enum
    {
        READING_FIRST_LINE,
        READING_HEADERS,
        READING_BODY,
        READING_CHUNK_SIZE,
        READING_CHUNK_BODY,
    };

public:
    SHttpReaderState()
    {
        Reset();
    }


    // Destroys the request data - Request object to be 
    // deleted by later stages
    ~SHttpReaderState()
    {
    }

    void Reset()
    {
        // reset reader state
        currState           = READING_FIRST_LINE;
        pCurrBodyPart       = NULL;
        currBodySize        = 0;
        currBodyRead        = 0;
        requestFullyRead    = false;
        pCurrRequest.Reset();
    }

public:
    bool ProcessBytes(char *&pStart, char *&pLast);

    bool ProcessCurrentLine();

    bool ProcessBodyData(char *&pStart, char *&pLast);

public:
    //! Current state
    int                 currState;

    //! Current header being read
    SStringStream       currHeaderLine;

    //! Current request being read
    SHttpRequest        pCurrRequest;

    //! Current body part being read
    SBodyPart *         pCurrBodyPart;

    //! Size of the current body or chunk
    unsigned            currBodySize;

    //! Number of bytes read in the current body or chunk
    unsigned            currBodyRead;

    //! Current line being accumulated
    SStringStream       currentLine;

    //! Set to true when a request has been read
    bool                requestFullyRead;
};

// Creates a new file io helper stage
SHttpReaderStage::SHttpReaderStage(const SString &name, int numThreads)
:
    SReaderStage(name, numThreads),
    pHandlerStage(NULL)
{
}

//! Default destructor
SHttpReaderStage::~SHttpReaderStage()
{
}

//! Creates a new reader state object
void *SHttpReaderStage::CreateStageData()
{
    return new SHttpReaderState();
}

//! Destroys reader state objects
void SHttpReaderStage::DestroyStageData(void *pReaderState)
{
    if (pReaderState != NULL)
        delete ((SHttpReaderState *)pReaderState);
}

//! Resets stage data when required
void SHttpReaderStage::ResetStageData(void *pData)
{
    ((SHttpReaderState *)pData)->Reset();
}

//! Handle the new assembled request
bool SHttpReaderStage::HandleRequest(SConnection *pConnection, void *pRequest)
{
    // send the request off to the handler stage
    return pHandlerStage->SendEvent_HandleNextRequest(pConnection, (SHttpRequest *)pRequest);
}

//! Process a bunch of bytes and try to assemble a request if enough bytes found
void *SHttpReaderStage::AssembleRequest(char *&pStart, char *&pLast, void *pState)
{
    SHttpReaderState *  pReaderState    = (SHttpReaderState *)pState;
    SHttpRequest *      pOut            = NULL;

    if (pReaderState->ProcessBytes(pStart, pLast))
    {
        if (pReaderState->requestFullyRead)
        {
            pOut = &(pReaderState->pCurrRequest);
        }
    }
    else
    {
        // we have an error??
    }
    return pOut;
}

bool SHttpReaderState::ProcessBytes(char *&pStart, char *&pLast)
{
    char *pCurr   = pStart;
    while (pCurr < pLast)
    {
        while (pCurr < pLast && 
                (currState == READING_FIRST_LINE ||
                currState == READING_HEADERS ||
                currState == READING_CHUNK_SIZE))
        {
            // TODO: do all the single-line bits in a single pass instead of two
            // go to the first CRLF or LF
            while ((pCurr < pLast) && (*pCurr != HttpUtils::CR) && (*pCurr != HttpUtils::LF)) pCurr++;

            currentLine << SString(pStart, pCurr - pStart);
            if (*pCurr == HttpUtils::CR)
            {
                pCurr += 2;
            }
            else if (*pCurr == HttpUtils::LF)
            {
                pCurr++;
            }
            else
            {
                // no more bytes left so leave 
                pStart = pCurr;
                return true;
            }

            if (!ProcessCurrentLine())
            {
                // error so just quit
                return false;
            }

            // point to start of next line
            pStart = pCurr;
        }

        if (currState == READING_BODY || currState == READING_CHUNK_BODY)
        {
            pStart = pCurr;
            // if (pLast <= pStart) return false;

            // if error in ready body data then quit
            if (!ProcessBodyData(pStart, pLast))
                return false;
            else if (requestFullyRead)
                return true;
        }
    }
    return false;
}

// Reads body data and returns the number of bytes processed
bool SHttpReaderState::ProcessBodyData(char *&pStart, char *&pLast)
{
    // length of the content
    size_t contLength = 0;
    if (currState == READING_BODY)
    {
        contLength = pCurrRequest.ContentLength();
        currBodySize = contLength;
    }
    else
    {
        contLength = currBodySize;
    }

    // how much do we need to read?
    size_t currBodyLeft = contLength - currBodyRead;

    // how much SHOULD we read?  we only want to read what ever of 
    // the body is left, despite how much is passed to this method
    size_t minLength    = ((pStart + currBodyLeft) < pLast) ? currBodyLeft : (pLast - pStart);
    if (minLength < 0)
    {
        // how can this be?
        assert("Why is minLength <= 0???" && false);
    }
    else if (minLength > 0)
    {
        if (pCurrBodyPart == NULL)
            pCurrBodyPart = pCurrRequest.NewBodyPart();
        pCurrBodyPart->AppendToBody(pStart, minLength);
    }

    currBodyRead    += minLength;  // increment what has been read
    if (currBodyRead == currBodySize)
    {
        if (currState == READING_BODY ||
            (currState == READING_CHUNK_BODY && currBodySize == 0))
        {
            // done add requests to the list of requests
            pCurrRequest.SetContentBody(pCurrBodyPart);
            requestFullyRead = true;
        }
    }

    // incrementing the curr buff position
    pStart += minLength;

    return true;
}

// Processes the current line
// returns 0 on success, 1 if success and complete
// -1 if failure
bool SHttpReaderState::ProcessCurrentLine()
{
    // create a request if none found
    // if (pCurrRequest == NULL) pCurrRequest = new SHttpRequest();

    SString currLine(currentLine.str());

    if (currState == READING_FIRST_LINE)
    {
        if (!pCurrRequest.ParseFirstLine(currLine))
            return false;

        SLogger::Get()->Log("DEBUG: %s %s %s\n", pCurrRequest.Method().c_str(),
                                                 pCurrRequest.Resource().c_str(),
                                                 pCurrRequest.Version().c_str());

        currHeaderLine.str("");
        currHeaderLine.clear();
        currState = READING_HEADERS;
    }
    else if (currState == READING_CHUNK_SIZE)
    {
        assert("Chunk reading not yet done." &&  false);

        // TODO: Set currBodySize to parsed chunk size 
        // and currBodyRead to 0
        currBodySize  = currBodyRead  = 0;

        // read the chunk body
        currState = READING_CHUNK_BODY;
    }
    else if (currState == READING_HEADERS)
    {
        if (!currLine.empty() && isspace(currLine[0]))
        {
            // append to previous line
            currHeaderLine << currLine;
        }
        else
        {
            // we have a valid header, so add the last header to our
            // list (if it wasnt empty)
            SString lastHeader(currHeaderLine.str());
            if ( ! lastHeader.empty())
            {
                SString hdrName, hdrValue;
                SHeaderTable &pHeaders = pCurrRequest.Headers();
                if (!pHeaders.ParseHeaderLine(lastHeader, hdrName, hdrValue))
                    return false;
                SLogger::Get()->Log("DEBUG: %s: %s\n", hdrName.c_str(), hdrValue.c_str());
            }

            currHeaderLine.str("");
            currHeaderLine.clear();

            // what do we do weith current line?

            if (currLine.empty())
            {
                currBodySize  = currBodyRead  = 0;

                // see if we are doing chunked encoding or not
                SString transferEncoding;
                if (pCurrRequest.Headers().HeaderIfExists("Transfer-Encoding", transferEncoding))
                {
                    if (strcasecmp(transferEncoding.c_str(), "chunked") == 0)
                    {
                        currState = READING_CHUNK_SIZE;
                    }
                    else
                    {
                        // TODO: Support for other transfer encodings
                        // Perhaps send all chunks to the TransferModule and
                        // let it do all the assembly?
                        return false;
                    }
                }
                else
                {
                    currState = READING_BODY;
                }
            }
            else
            {
                currHeaderLine << currLine;
            }
        }
    }

    currentLine.str("");
    currentLine.clear();

    return true;
}

