//*****************************************************************************
/*!
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
void JsonTokenizer::Begin(std::istream &instream)
{
    Reset();

    input       = &instream;
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
 *  \brief  Gets the next character, -ve if eof.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
int JsonTokenizer::GetChar()
{
    if (lastChar >= 0)
    {
        int out = lastChar;
        lastChar = -1;
        return out;
    }

    int ch = input->get();
    if (input->bad() || input->eof())
        return -1;
    return ch;
}

//*****************************************************************************
/*!
 *  \brief  Unget a character.
 *
 *  \version
 *      - S Panyam     02/02/2009
 *        Created.
 *
 *****************************************************************************/
void JsonTokenizer::UngetChar(int ch)
{
    lastChar = ch;
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
    int     ch  = GetChar();

    while (ch >= 0 && isspace(ch))
    {
        ch = GetChar();
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
        ch = GetChar();
        while (ch != delim)
        {
            if (ch < 0)
            {
                tokText = str.str();
                return JT_ERROR;
            }
            if (ch == '\\')
            {
                int nextch = GetChar();

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
                    case '\\': str << '\\'; break;
                    default: str << '\\' << (char)nextch;
                }
            }
            else
            {
                str << (char)ch;
            }
            ch = GetChar();
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
            ch = GetChar();
        }
        UngetChar(ch);

        tokText = str.str();
        return JT_DIGITS;
    }
    else if (isalpha(ch) || ch == '_')
    {
        // an identifier
        std::stringstream str;
        str << char(ch);
        ch = GetChar();

        while (ch >= 0 && (isalnum(ch) || ch == '_'))
        {
            str << char(ch);
            ch = GetChar();
        }

        UngetChar(ch);
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
    input       = NULL;
    tokText     = "";
    lastChar    = -1;
    laToken     = JT_NONE;
}

