//*****************************************************************************
/*!
 *  \file   json.h
 *
 *  \brief  JSON Utilities
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SJSON_H_
#define _SJSON_H_

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <iterator>
#include "utils/refcount.h"
#include "utils/membuff.h"

class JsonInputStream ;

//*****************************************************************************
/*!
 *  \class  JsonNodeType
 *
 *  \brief  JSON Node Types
 *
 *****************************************************************************/
typedef enum
{
    JNT_NULL,       // A null value
    JNT_BOOL,       // A bool value
    JNT_INT,        // A integer value
    JNT_DOUBLE,     // A floating point value
    JNT_STRING,     // A string value
    JNT_LIST,       // list of items
    JNT_OBJECT,     // Collection of name-value pairs
} JsonNodeType;

class JsonNode;

typedef CSmartPtr<JsonNode> JsonNodePtr;

//*****************************************************************************
/*!
 *  \class  SJsonNode
 *
 *  \brief  A JSON Node
 *
 *****************************************************************************/
class JsonNode
{
public:
    // Destructor
    virtual ~JsonNode();

    // Return node type
    virtual JsonNodeType    Type() const;

    // Clears all children
    virtual void            Clear();

    // Tells if node is empty
    virtual bool            IsEmpty();

    // Return node value - only valid for leaf types
    template<typename T> T  Value() const;

    // Return child count
    virtual unsigned        Size();

    // Add a child node if possible
    virtual bool            Add(const JsonNodePtr &child, int index = -1);

    // Add a named value
    virtual bool            Set(const std::string &name, const JsonNodePtr &child);

    // Get a child node by index
    virtual JsonNodePtr     Get(unsigned index) const;

    // Get a child node by name
    virtual JsonNodePtr     Get(const std::string &name) const;

    // Get a child node by index
    template <typename T>
    T Get(unsigned index, const T & valOnInvalid) const;

    // Get a child node by name
    template <typename T>
    T Get(const std::string &name, const T & valOnInvalid) const;

    // Tells if a certain attrib exists (only valid on object nodes)
    virtual bool            Contains(const std::string &attr);

protected:
    JsonNode() { }
};

template<> bool         JsonNode::Value() const;
template<> int          JsonNode::Value() const;
template<> double       JsonNode::Value() const;
template<> std::string  JsonNode::Value() const;

// Get a child node by index
template <> int         JsonNode::Get(unsigned index, const int & valOnInvalid) const;
template <> bool        JsonNode::Get(unsigned index, const bool & valOnInvalid) const;
template <> double      JsonNode::Get(unsigned index, const double & valOnInvalid) const;
template <> std::string JsonNode::Get(unsigned index, const std::string &valOnInvalid) const;

template <> int         JsonNode::Get(const std::string &name, const int & valOnInvalid) const;
template <> bool        JsonNode::Get(const std::string &name, const bool & valOnInvalid) const;
template <> double      JsonNode::Get(const std::string &name, const double & valOnInvalid) const;
template <> std::string JsonNode::Get(const std::string &name, const std::string &valOnInvalid) const;

//*****************************************************************************
/*!
 *  \class  JsonTypedNode
 *
 *  \brief  JsonNode with ONLY its type defined
 *
 *****************************************************************************/
template <JsonNodeType NT>
class JsonTypedNode : public JsonNode
{
public:
    // Return node type
    JsonNodeType    Type() const { return NT; }
};

//*****************************************************************************
/*!
 *  \class  JsonBasicNode
 *
 *  \brief  JsonNodes for basic non-collection values
 *
 *****************************************************************************/
template <JsonNodeType NT, typename VT>
class JsonBasicNode : public JsonTypedNode<NT>
{
public:
    //! Constructor
    JsonBasicNode(const VT &value) : nodeValue(value) { }

    //! Return node value
    VT Value() const { return nodeValue; }

protected:
    VT nodeValue;
};

//*****************************************************************************
/*!
 *  \class  JsonContainerNode
 *
 *  \brief  An list node - name/value collection.
 *
 *****************************************************************************/
class JsonListNode :
    public JsonTypedNode<JNT_LIST>,
    private std::vector<JsonNodePtr>
{
public:
    // Constructor
    JsonListNode();

    // Destructor
    virtual ~JsonListNode();

    // Clears all children
    virtual void            Clear();

    // Tells if node is empty
    virtual bool            IsEmpty();

    // Add a child node if possible
    virtual bool            Add(const JsonNodePtr &child, int index = -1);

    // Return child count
    virtual unsigned        Size();

    // Get a child node by index
    virtual JsonNodePtr     Get(unsigned index) const;

protected:
    using std::vector<JsonNodePtr>::empty;
    using std::vector<JsonNodePtr>::size;
    using std::vector<JsonNodePtr>::push_back;
    using std::vector<JsonNodePtr>::clear;
};

//*****************************************************************************
/*!
 *  \class  JsonObjectNode
 *
 *  \brief  An object node - name/value collection.
 *
 *****************************************************************************/
class JsonObjectNode : 
    public JsonTypedNode<JNT_OBJECT>,
    private std::map<std::string, JsonNodePtr>
{
protected:
    typedef std::map<std::string, JsonNodePtr>   JsonNodeMap;

public:
    // Constructor
    JsonObjectNode();

    // Destructor
    virtual ~JsonObjectNode();

    // Clears all children
    virtual void            Clear();

    // Tells if node is empty
    virtual bool            IsEmpty();

    // Return child count
    virtual unsigned        Size();

    // Add a named value
    virtual bool            Set(const std::string &name, const JsonNodePtr &child);

    // Get a child node by name
    virtual JsonNodePtr     Get(const std::string &name) const;

    // Tells if a certain attrib exists (only valid on object nodes)
    virtual bool            Contains(const std::string &attr);

public:
    using JsonNodeMap::iterator;
    using JsonNodeMap::begin;
    using JsonNodeMap::end;

protected:
    using JsonNodeMap::empty;
    using JsonNodeMap::size;
    using JsonNodeMap::clear;
    using JsonNodeMap::insert;
    using JsonNodeMap::find;
};

//*****************************************************************************
/*!
 *  \class  JsonNodeFactory
 *
 *  \brief  Creates json nodes.
 *
 *****************************************************************************/
class JsonNodeFactory
{
public:
    // Create a bool node
    static JsonNodePtr  BoolNode(bool value);

    // Create an integer node
    static JsonNodePtr  IntNode(int value);

    // create a floating point number node
    static JsonNodePtr  DoubleNode(double value);

    // create a string node
    static JsonNodePtr  StringNode(const std::string &str);

    // create a list node
    static JsonNodePtr  ListNode();

    // create an object node
    static JsonNodePtr  ObjectNode();
};

//*****************************************************************************
/*!
 *  \class  JsonFormatter
 *
 *  \brief  Writes formatted Json to an output stream.
 *
 *****************************************************************************/
class JsonFormatter
{
public:
    virtual ~JsonFormatter() { }
    virtual std::ostream &Format(std::ostream &, const JsonNodePtr &node)   =   0;

    static  std::ostream &FormatString(std::ostream &, const std::string &);
};

//*****************************************************************************
/*!
 *  \class  DefaultJsonFormatter
 *
 *  \brief  Defualt JsonFormatter implementation - allows for indentation.
 *
 *****************************************************************************/
class DefaultJsonFormatter : public JsonFormatter
{
public:
    // Format json 
    virtual std::ostream &Format(std::ostream &, const JsonNodePtr &node);

    // Format json
    virtual std::ostream &FormatExt(std::ostream &, const JsonNodePtr &node, bool appendNewline = false);
};

//*****************************************************************************
/*!
 *  \class  AMFFormatter
 *
 *  \brief  Writes json as AMF fragment - the Action Message Format - a
 *  very efficient message format for encoding objects.
 *
 *****************************************************************************/
class AMFFormatter : public JsonFormatter
{
public:
    const static char MARKER_UNDEF;
    const static char MARKER_NULL;
    const static char MARKER_FALSE;
    const static char MARKER_TRUE;
    const static char MARKER_INT;
    const static char MARKER_DOUBLE;
    const static char MARKER_STRING;
    const static char MARKER_XMLDOC;
    const static char MARKER_DATE;
    const static char MARKER_ARRAY;
    const static char MARKER_OBJECT;
    const static char MARKER_XML;
    const static char MARKER_BYTEARRAY;

public:
    //! Format json 
    virtual std::ostream &  Format(std::ostream &, const JsonNodePtr &node);
    virtual void            Format(SMemBuff *pMem, const JsonNodePtr &node);

public:
    static void encodeUTF8(const std::string &input, SMemBuff *pMem);
    static int  encodeInt(int input, unsigned char *bytes);
};

//====================      Json Builder    =======================//

//*****************************************************************************
/*!
 *  \class  JsonBuilder
 *
 *  \brief  Builds Json trees out of input stream.
 *
 *****************************************************************************/
class JsonBuilder
{
public:
    virtual ~JsonBuilder() { }
    virtual JsonNodePtr     Build(JsonInputStream *pStream) = 0;
};

//*****************************************************************************
/*!
 *  \class  DefaultJsonBuilder
 *
 *  \brief  Default builder for Json.
 *
 *****************************************************************************/
class DefaultJsonBuilder : public JsonBuilder
{
public:
    virtual ~DefaultJsonBuilder() { }
    virtual JsonNodePtr     Build(JsonInputStream *pStream);
};


#endif

