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
 *  \file   tokenizer.cpp
 *
 *  \brief  JSON Tokenizer/Scanner.
 *
 *  \version
 *      - S Panyam      02/02/2009
 *        Created
 *
 *****************************************************************************/

#include <assert.h>
#include <sstream>
#include <iostream>

#include "json/tokenizer.h"

using std::endl;
using std::cerr;

//*****************************************************************************
/*!
 *  \brief  Begin tokenizing.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
void JsonTokenizer::Begin(JsonInputStream *pStream)
{
    Reset();

    pInputStream = pStream;
}

//*****************************************************************************
/*!
 *  \brief  Returns the corresponding token text.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
const std::string & JsonTokenizer::TokenText() const
{
    return tokText;
}

//*****************************************************************************
/*!
 *  \brief  Return the next token.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonTokenizer::JsonToken JsonTokenizer::NextToken()
{
    // see what the look ahead is (if any)
    JsonToken jt = laToken;

    laToken = JT_NONE;

    if (jt == JT_NONE)
    {
        jt = RealToken();
    }

    if (jt != JT_DIGITS && jt != JT_MINUS)
    {
        return jt;
    }

    bool                founddot(false);
    std::string         toktext;
    std::stringstream   str("");

    if (jt == JT_MINUS)
    {
        str << "-";
        jt = RealToken();

        if (jt != JT_DIGITS)
        {
            std::cerr << "Expected JT_DIGITS after JT_MINUS.  Found: " << jt << std::endl;
            return JT_ERROR;
        }
    }

    // write the number into the stream
    str << TokenText();

    // Get the next token
    jt      = RealToken();

    if (jt == JT_DOT)
    {
        str << ".";
        jt      = RealToken();

        if (jt != JT_DIGITS)
        {
            std::cerr << "Expected JT_DIGITS after JT_DOT.  Found: " << jt << std::endl;
            return JT_ERROR;
        }

        founddot = true;

        str << TokenText();
        jt      = RealToken();
    }

    toktext = TokenText();
    if (jt == JT_IDENT && (toktext == "e" || toktext == "E"))
    {
        str << TokenText();
        jt      = RealToken();

        if (jt == JT_PLUS || jt == JT_MINUS)
        {
            str << (jt == JT_PLUS ? "+" : "-");
            jt = RealToken();
        }

        if (jt != JT_DIGITS)
        {
            std::cerr << "Expected JT_DIGITS after 'e'.  Found: " << jt << std::endl;
            return JT_ERROR;
        }

        str << TokenText();
        tokText = str.str();
        return JT_DOUBLE;
    }
    else
    {
        tokText = str.str();
        laToken = jt;
        return founddot ? JT_DOUBLE : JT_INT;
    }

    return jt;
}

//*****************************************************************************
/*!
 *  \brief  Return the next real token encountered.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonTokenizer::JsonToken JsonTokenizer::RealToken()
{
    int     ch  = pInputStream->GetChar();

    while (ch >= 0 && isspace(ch))
    {
        ch = pInputStream->GetChar();
    }

    if (ch < 0)
        return JT_EOF;

    switch (ch)
    {
        case ',':
            return JT_COMA;
        case ':':
            return JT_COLON;
        case '{':
            return JT_OPEN_BRACE;
        case '}':
            return JT_CLOSE_BRACE;
        case '[':
            return JT_OPEN_SQUARE;
        case ']':
            return JT_CLOSE_SQUARE;
        case '.':
            return JT_DOT;
        case '-':
            return JT_MINUS;
        case '+':
            return JT_PLUS;
    }

    if (ch == '\'' || ch == '"')
    {
        std::stringstream str;
        char delim = ch;
        ch = pInputStream->GetChar();
        while (ch != delim)
        {
            if (ch < 0)
            {
                tokText = str.str();
                return JT_ERROR;
            }
            if (ch == '\\')
            {
                int nextch = pInputStream->GetChar();

                if (nextch < 0)
                {
                    tokText = str.str();
                    return JT_ERROR;
                }

                switch (nextch)
                {
                    case 'n': str << '\n'; break;
                    case 'r': str << '\r'; break;
                    case 'f': str << '\f'; break;
                    case 'b': str << '\b'; break;
                    case 't': str << '\t'; break;
                    case 'v': str << '\v'; break;
                    case '/': str << '/'; break;
                    case '\\': str << '\\'; break;
                    default: str << '\\' << (char)nextch;
                }
            }
            else
            {
                str << (char)ch;
            }
            ch = pInputStream->GetChar();
        }
        tokText = str.str();
        return JT_STRING;
    }
    else if (isdigit(ch))
    {
        std::stringstream    str;
        while (ch >= 0 && isdigit(ch))
        {
            str << char(ch);
            ch = pInputStream->GetChar();
        }
        pInputStream->UngetChar(ch);

        tokText = str.str();
        return JT_DIGITS;
    }
    else if (isalpha(ch) || ch == '_')
    {
        // an identifier
        std::stringstream str;
        str << char(ch);
        ch = pInputStream->GetChar();

        while (ch >= 0 && (isalnum(ch) || ch == '_'))
        {
            str << char(ch);
            ch = pInputStream->GetChar();
        }

        pInputStream->UngetChar(ch);
        tokText = str.str();

        if (tokText == "true")
            return JT_TRUE;
        else if (tokText == "false")
            return JT_FALSE;
        else if (tokText == "null")
            return JT_NULL;

        return JT_IDENT;
    }

    return JT_ERROR;
}

//*****************************************************************************
/*!
 *  \brief  Reset the scanner.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
void JsonTokenizer::Reset()
{
    pInputStream    = NULL;
    tokText         = "";
    laToken         = JT_NONE;
}

