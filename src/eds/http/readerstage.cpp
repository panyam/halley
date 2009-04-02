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
 *  \brief  The stage that asynchronously reads http requests of the
 *  socket.
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
#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

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
    SHttpReaderState() :
        currState(READING_FIRST_LINE),
        pCurrRequest(NULL),
        pCurrBodyPart(NULL),
        currBodySize(0),
        currBodyRead(0) { }


    // Destroys the request data - Request object to be 
    // deleted by later stages
    ~SHttpReaderState() { }

public:
    bool ProcessCurrentLine(std::list<SHttpRequest *> & requests);

    size_t ProcessBodyData(const char *buffer, size_t len, std::list<SHttpRequest *> & requests);

    bool ProcessBytes(const char *buffer, size_t len, std::list<SHttpRequest *> & requests);

public:
    //! Current state
    int                 currState;

    //! Current header being read
    SStringStream   currHeaderLine;

    //! Current request being read
    SHttpRequest *      pCurrRequest;

    //! Current body part being read
    SBodyPart *         pCurrBodyPart;

    //! Size of the current body or chunk
    unsigned            currBodySize;

    //! Number of bytes read in the current body or chunk
    unsigned            currBodyRead;

    //! Current line being accumulated
    SStringStream   currentLine;
};

// Creates a new file io helper stage
SHttpReaderStage::SHttpReaderStage(int numThreads)
:
    SStage(numThreads),
    pHandlerStage(NULL)
{
}

// Open a file handle
void SHttpReaderStage::SetHandlerStage(SHttpHandlerStage *pHandler)
{
    pHandlerStage = pHandler;
}

// Read bytes
void SHttpReaderStage::ReadSocket(SConnection *pConnection)
{
    QueueEvent(SEvent(EVT_BYTES_RECIEVED, pConnection));
}

//! Handles "read request" events.
//
// Will call the RequestHandler stage when a complete request has been
// read.
void SHttpReaderStage::HandleEvent(const SEvent &event)
{
    // The connection currently being processed
    SConnection *pConnection    = (SConnection *)(event.pSource);
    void *pStageData            = pConnection->GetStageData(this);
    if (pStageData == NULL)
    {
        pStageData = new SHttpReaderState();
        pConnection->SetStageData(this, pStageData);
    }
    SHttpReaderState *pReaderState = (SHttpReaderState *)pStageData;

    const int MAXBUF = 1024;
    char buffer[MAXBUF + 1];
    std::list<SHttpRequest *>   requests;
    int len = read(pConnection->Socket(), buffer, MAXBUF);

    while (len > 0)
    {
        buffer[len] = 0;
        // consume all bytes
        if (pReaderState->ProcessBytes(buffer, len, requests))
        {
            while ( ! requests.empty())
            {
                SHttpRequest *pRequest = requests.front();
                requests.pop_front();

                // send the request off to the handler stage
                pHandlerStage->HandleRequest(pConnection, pRequest);
            }
        }
        else
        {
            std::cerr << "ERROR: Error in request: [" << errno << "]: " << strerror(errno) << std::endl << std::endl;
            pConnection->Close();
        }
        len = read(pConnection->Socket(), buffer, MAXBUF);
    }

    if (len < 0)
    {
        std::cerr << "WARNING: Socket Reading Complete: [" << errno << "]: " << strerror(errno) << std::endl << std::endl;
        // pConnection->Close();
    }
}

//! Process a bunch of bytes
bool SHttpReaderState::ProcessBytes(const char *buffer, size_t len, std::list<SHttpRequest *> & requests)
{
    const char *pStart  = buffer;
    const char *pCurr   = buffer;
    const char *pLast   = buffer + len;
    while (true)
    {
        while (pCurr < pLast && 
                (currState == READING_FIRST_LINE ||
                currState == READING_HEADERS ||
                currState == READING_CHUNK_SIZE))
        {
            // go to the first CRLF or LF
            while ((pCurr < pLast) && (*pCurr != CR) && (*pCurr != LF)) pCurr++;

            currentLine << SString(pStart, pCurr - pStart);
            if (*pCurr == CR)
            {
                pCurr += 2;
            }
            else if (*pCurr == LF)
            {
                pCurr++;
            }
            else
            {
                // no more bytes left so leave 
                return true;
            }

            if (!ProcessCurrentLine(requests))
            {
                // error so just quit
                return false;
            }

            // point to start of next line
            pStart = pCurr;
        }

        if (currState == READING_BODY || currState == READING_CHUNK_BODY)
        {
            size_t len      = pLast > pStart ? pLast - pStart : 0;
            size_t numBytes = ProcessBodyData(pStart, len, requests);
            if (numBytes == len)
                return true;

            // otherwise start again with the next request
            currState = READING_FIRST_LINE;
        }
    }
}

// Reads body data and returns the number of bytes processed
size_t SHttpReaderState::ProcessBodyData(const char *buffer, size_t len, std::list<SHttpRequest *> & requests)
{
    size_t contLength = 0;
    if (currState == READING_BODY)
    {
        contLength = pCurrRequest->ContentLength();
        currBodySize = contLength;
    }
    else
    {
        contLength = currBodySize;
    }

    size_t currBodyLeft = contLength - currBodyRead;
    size_t minLength    = currBodyLeft < len ? currBodyLeft : len;

    if (minLength > 0)
    {
        if (pCurrBodyPart == NULL)
            pCurrBodyPart = pCurrRequest->NewBodyPart();
        pCurrBodyPart->AppendToBody(buffer, minLength);
    }

    currBodyRead += minLength;
    if (currBodyRead == currBodySize)
    {
        if (currState == READING_BODY ||
            (currState == READING_CHUNK_BODY && currBodySize == 0))
        {
            // done add requests to the list of requests
            pCurrRequest->SetContentBody(pCurrBodyPart);
            requests.push_back(pCurrRequest);
            pCurrRequest = NULL;
            pCurrBodyPart = NULL;
            currState = READING_FIRST_LINE;
        }
    }

    // return true if more data is available
    return minLength;
}

// Processes the current line
// returns 0 on success, 1 if success and complete
// -1 if failure
bool SHttpReaderState::ProcessCurrentLine(std::list<SHttpRequest *> & requests)
{
    // create a request if none found
    if (pCurrRequest == NULL) pCurrRequest = new SHttpRequest();

    SString currLine(currentLine.str());

    if (currState == READING_FIRST_LINE)
    {
        if (!pCurrRequest->ParseFirstLine(currLine))
            return false;

        std::cout << pCurrRequest->Method() << " "
                  << pCurrRequest->Resource() << " "
                  << pCurrRequest->Version() << endl;

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
                SHeaderTable &pHeaders = pCurrRequest->Headers();
                if (!pHeaders.ParseHeaderLine(lastHeader, hdrName, hdrValue))
                    return false;
                std::cout << hdrName << ": " << hdrValue << std::endl;
            }

            currHeaderLine.str("");
            currHeaderLine.clear();

            // what do we do weith current line?

            if (currLine.empty())
            {
                currBodySize  = currBodyRead  = 0;

                // see if we are doing chunked encoding or not
                SString transferEncoding;
                if (pCurrRequest->Headers().HeaderIfExists("Transfer-Encoding", transferEncoding))
                {
                    if (transferEncoding == "chunked")
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

