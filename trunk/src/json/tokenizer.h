//*****************************************************************************
/*!
 *  \file   tokenizer.h
 *
 *  \brief  JSON tokenizer Utilities
 *
 *  \version
 *      - S Panyam      02/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _GPJSON_TOKENIZER_H_
#define _GPJSON_TOKENIZER_H_

#include <string>
#include <iterator>

//*****************************************************************************
/*!
 *  \class JsonInputStream
 *
 *  \brief Provides char getting and ungetting to the tokenizer.
 *
 *****************************************************************************/
class JsonInputStream
{
public:
    virtual ~JsonInputStream() { }
    virtual int     GetChar()           = 0;
    virtual void    UngetChar(int ch)   = 0;
};

//*****************************************************************************
/*!
 *  \class DefaultJsonInputStream
 *
 *  \brief
 *  A JSON input stream interface that caters for arbitrary input
 *  iterators.
 *
 *****************************************************************************/
template <class InputIterator>
class DefaultJsonInputStream : public JsonInputStream
{
public:
    DefaultJsonInputStream(InputIterator start, InputIterator end)
    {
        pInputStart = start;
        pInputEnd   = end;
        lastChar    = -1;
    }

    //! Gets the next character
    virtual int     GetChar()
    {
        int out = -1;
        if (lastChar >= 0)
        {
            out = lastChar;
            lastChar = -1;
            return out;
        }

        // has iterator reached the end?
        if (pInputStart == pInputEnd)
            return -1;

        out = *pInputStart;
        pInputStart++;

        return out;
    }

    //! Ungets the character
    virtual void    UngetChar(int ch)
    {
        lastChar = ch;
    }

protected:
    InputIterator   pInputStart;
    InputIterator   pInputEnd;
    int             lastChar;
};


//*****************************************************************************
/*!
 *  \class JsonTokenizer
 *
 *  \brief Returns tokens from a JSON stream.
 *
 *****************************************************************************/
class JsonTokenizer
{
public:
    typedef enum
    {
        JT_NONE             = -2,
        JT_ERROR            = -1,
        JT_EOF              = 0,
        JT_OPEN_BRACE,      // "{"
        JT_CLOSE_BRACE,     // "}"
        JT_OPEN_SQUARE,     // "["
        JT_CLOSE_SQUARE,    // "]"
        JT_COMA,            // ","
        JT_COLON,           // ":"
        JT_NULL,            // null
        JT_TRUE,            // true
        JT_FALSE,           // false
        JT_INT,             // "integer"
        JT_DOUBLE,          // "double"
        JT_IDENT,           // "identifier"
        JT_STRING,          // "string"
        JT_DOT,             // "."
        JT_PLUS,            // "+"
        JT_MINUS,           // "-"
        JT_DIGITS,          // [0-9]+
        JT_COUNT
    } JsonToken;

public:
    virtual ~JsonTokenizer()    { }
    virtual void                Begin(JsonInputStream *);
    virtual JsonToken           NextToken();
    virtual const std::string & TokenText() const;

protected:
    void        Reset();
    JsonToken   RealToken();

private:
    JsonInputStream *   pInputStream;
    std::string         tokText;
    JsonToken           laToken;
};

#endif

