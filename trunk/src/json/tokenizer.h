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
    virtual void                Begin(std::istream &);
    virtual JsonToken           NextToken();
    virtual const std::string & TokenText() const;

protected:
    void        Reset();
    int         GetChar();
    void        UngetChar(int ch);
    JsonToken   RealToken();

private:
    int             lastChar;
    std::istream *  input;
    std::string     tokText;
    JsonToken       laToken;
};

#endif

