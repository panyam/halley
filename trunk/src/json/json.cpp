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
 *  \file   gpjson.cpp
 *
 *  \brief  JSON Utility Classes
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include <assert.h>
#include <string>

#include "json/json.h"
#include "json/tokenizer.h"

using std::endl;
using std::cerr;

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNode::~JsonNode()
{
}

//*****************************************************************************
/*!
 *  \brief  Convert's a node's value to a bool otherwise returns the
 *  fallback value.
 *
 *  Also if the required value is missing as a bool, it is searched for as
 *  a string to see if it exists.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool JsonNode::ValueToBool(const JsonNodePtr &node, const bool &valOnInvalid)
{
    if (!node)
        return valOnInvalid;

    if (node->Type() == JNT_STRING)
    {
        std::string value(static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(node.Data())->Value());
        if (value == "true")
            return true;
        else if (value == "false")
            return false;
        return valOnInvalid;
    }

    if (node->Type() != JNT_BOOL)
        return valOnInvalid;

    return static_cast<const JsonBasicNode<JNT_BOOL, bool> *>(node.Data())->Value();
}

//*****************************************************************************
/*!
 *  \brief  Convert's a node's value to a int otherwise returns the
 *  fallback value.
 *
 *  Also if the required value is missing as a int, it is searched for as
 *  a string to see if it exists.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int JsonNode::ValueToInt(const JsonNodePtr &node, const int &valOnInvalid)
{
    if (!node)
        return valOnInvalid;

    if (node->Type() == JNT_STRING)
    {
        std::string value(static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(node.Data())->Value());
        return atoi(value.c_str());
        // return valOnInvalid;
    }

    if (node->Type() != JNT_INT)
        return valOnInvalid;

    return static_cast<const JsonBasicNode<JNT_INT, int> *>(node.Data())->Value();
}

//*****************************************************************************
/*!
 *  \brief  Convert's a node's value to a double otherwise returns the
 *  fallback value.
 *
 *  Also if the required value is missing as a double, it is searched for as
 *  a string to see if it exists.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
double JsonNode::ValueToDouble(const JsonNodePtr &node, const double &valOnInvalid)
{
    if (!node)
        return valOnInvalid;

    if (node->Type() == JNT_STRING)
    {
        std::string value(static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(node.Data())->Value());
        return atoi(value.c_str());
        // return valOnInvalid;
    }

    if (node->Type() != JNT_DOUBLE)
        return valOnInvalid;

    return static_cast<const JsonBasicNode<JNT_DOUBLE, double> *>(node.Data())->Value();
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given index.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  unsigned index              Index of the node
 *  \param  const bool & valOnInvalid   Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> bool
JsonNode::Get(unsigned index, const bool & valOnInvalid) const
{
    JsonNodePtr node = Get(index);
    return ValueToBool(node, valOnInvalid);
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given hash.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  const std::string key       Key of the node
 *  \param  const bool & valOnInvalid   Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> bool
JsonNode::Get(const std::string &key, const bool & valOnInvalid) const
{
    JsonNodePtr node = Get(key);
    return ValueToBool(node, valOnInvalid);
}


//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given index.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  unsigned index              Index of the node
 *  \param  const int & valOnInvalid    Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> int
JsonNode::Get(unsigned index, const int & valOnInvalid) const
{
    JsonNodePtr node = Get(index);
    return ValueToInt(node, valOnInvalid);
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given hash.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  const std::string key       Key of the node
 *  \param  const int & valOnInvalid    Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> int
JsonNode::Get(const std::string &key, const int & valOnInvalid) const
{
    JsonNodePtr node = Get(key);
    return ValueToInt(node, valOnInvalid);
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given index.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  unsigned index              Index of the node
 *  \param  const double & valOnInvalid Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> double
JsonNode::Get(unsigned index, const double & valOnInvalid) const
{
    JsonNodePtr node = Get(index);
    return ValueToDouble(node, valOnInvalid);
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given hash.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  const std::string key           Key of the node
 *  \param  const double & valOnInvalid     Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> double
JsonNode::Get(const std::string &key, const double & valOnInvalid) const
{
    JsonNodePtr node = Get(key);
    return ValueToDouble(node, valOnInvalid);
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given index.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  unsigned index                      Index of the node
 *  \param  const std::string & valOnInvalid    Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> std::string
JsonNode::Get(unsigned index, const std::string & valOnInvalid) const
{
    JsonNodePtr node = Get(index);

    if (!node || node->Type() != JNT_STRING)
        return valOnInvalid;

    return static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(node.Data())->Value();
}

//*****************************************************************************
/*!
 *  \brief  Return value of the node at a given hash.  If the node is
 *  invalid, the specific valOnInvalid is returned instead.
 *
 *  \param  const std::string key           Key of the node
 *  \param  const double & valOnInvalid     Value to return on invalid node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template <> std::string
JsonNode::Get(const std::string &key, const std::string & valOnInvalid) const
{
    JsonNodePtr node = Get(key);

    if (!node || node->Type() != JNT_STRING)
        return valOnInvalid;

    return static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(node.Data())->Value();
}


//*****************************************************************************
/*!
 *  \brief  Return bool value of the node (asserts if node is NOT a
 *  JNT_BOOL)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template<> bool
JsonNode::Value() const
{
    assert(Type() == JNT_BOOL);

    return static_cast<const JsonBasicNode<JNT_BOOL, bool> *>(this)->Value();
}


//*****************************************************************************
/*!
 *  \brief  Return int value of the node (asserts if node is NOT a
 *  JNT_INT)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template<> int
JsonNode::Value() const
{
    assert(Type() == JNT_INT);

    return static_cast<const JsonBasicNode<JNT_INT, int> *>(this)->Value();
}


//*****************************************************************************
/*!
 *  \brief  Return double value of the node (asserts if node is NOT a
 *  JNT_DOUBLE)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template<> double
JsonNode::Value() const
{
    assert(Type() == JNT_DOUBLE);

    return static_cast<const JsonBasicNode<JNT_DOUBLE, double> *>(this)->Value();
}

//*****************************************************************************
/*!
 *  \brief  Return string value of the node (asserts if node is NOT a
 *  JNT_STRING)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
template<> std::string
JsonNode::Value() const
{
    assert(Type() == JNT_STRING);

    return static_cast<const JsonBasicNode<JNT_STRING, std::string> *>(this)->Value();
}

//*****************************************************************************
/*!
 *  \brief  Return node type
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodeType
JsonNode::Type() const
{
    return JNT_NULL;
}

//*****************************************************************************
/*!
 *  \brief  Clears the collection
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void
JsonNode::Clear()
{
}

//*****************************************************************************
/*!
 *  \brief  Returns if collection is empty
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonNode::IsEmpty()
{
    return true;
}

//*****************************************************************************
/*!
 *  \brief  Return child count
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
unsigned JsonNode::Size() const
{
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Add a node by index (only valid on list nodes)
 *
 *  \return true if node added, false otherwise
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonNode::Add(const JsonNodePtr &child, int index)
{
    return false;
}

//*****************************************************************************
/*!
 *  \brief  Add a node by name (only valid on object nodes)
 *
 *  \return true if node added, false otherwise
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonNode::Set(const std::string &name, const JsonNodePtr &child)
{
    return false;
}

//*****************************************************************************
/*!
 *  \brief  Return a child node by index (only valid on list nodes)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNode::Get(unsigned index) const
{
    return JsonNodePtr();
}

//*****************************************************************************
/*!
 *  \brief  Return child node by name (only valid on object nodes)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNode::Get(const std::string &name) const
{
    return JsonNodePtr();
}

//*****************************************************************************
/*!
 *  \brief  Tells if a specific attribute exists.
 *
 *  \param  const std::string &name Name of the attribute to check.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonNode::Contains(const std::string &name)
{
    return false;
}

//==================    Collection Nodes    ==================//

//*****************************************************************************
/*!
 *  \brief  Consturctor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonListNode::JsonListNode()
{
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonListNode::~JsonListNode()
{
    clear();
}

//*****************************************************************************
/*!
 *  \brief  Clears the collection
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void
JsonListNode::Clear()
{
    clear();
}

//*****************************************************************************
/*!
 *  \brief  Returns if collection is empty
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonListNode::IsEmpty()
{
    return empty();
}

//*****************************************************************************
/*!
 *  \brief  Return child count
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
unsigned JsonListNode::Size() const
{
    return size();
}

//*****************************************************************************
/*!
 *  \brief  Add a node by index (only valid on list nodes)
 *
 *  \return true if node added, false otherwise
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonListNode::Add(const JsonNodePtr &child, int index)
{
    if (index < 0)
        push_back(child);
    else
        insert(begin() + index, child);

    return true;
}

//*****************************************************************************
/*!
 *  \brief  Return a child node by index (only valid on list nodes)
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonListNode::Get(unsigned index) const
{
    return *(begin() + index);
}

//*****************************************************************************
/*!
 *  \brief  Consturctor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonObjectNode::JsonObjectNode()
{
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonObjectNode::~JsonObjectNode()
{
    clear();
}

//*****************************************************************************
/*!
 *  \brief  Clears the collection
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void
JsonObjectNode::Clear()
{
    clear();
}

//*****************************************************************************
/*!
 *  \brief  Returns if collection is empty
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonObjectNode::IsEmpty()
{
    return empty();
}

//*****************************************************************************
/*!
 *  \brief  Return child count
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
unsigned JsonObjectNode::Size() const
{
    return size();
}

//*****************************************************************************
/*!
 *  \brief  Add a node by name.  Replaces existing values
 *
 *  \return true if node added, false otherwise
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonObjectNode::Set(const std::string &name, const JsonNodePtr &child)
{
    JsonNodeMap::iterator iter = find(name);

    if (iter == end())
    {
        insert(std::pair<std::string, JsonNodePtr>(name, child));
    }
    else
    {
        // Must be an easier way to replace without resorting 
        // to the [] operator
        erase(name);
        insert(std::pair<std::string, JsonNodePtr>(name, child));
    }

    return true;
}

//*****************************************************************************
/*!
 *  \brief  Return child node by name.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonObjectNode::Get(const std::string &name) const
{
    JsonNodeMap::const_iterator iter = find(name);

    if (iter == end())
        return JsonNodePtr();

    return iter->second;
}

//*****************************************************************************
/*!
 *  \brief  Tells if a specific attribute exists.
 *
 *  \param  const std::string &name Name of the attribute to check.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
bool
JsonObjectNode::Contains(const std::string &attrib)
{
    JsonNodeMap::iterator iter = find(attrib);

    return iter != end();
}

//====================  Json Node Factory =======================//

//*****************************************************************************
/*!
 *  \brief  Create a boolean node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::BoolNode(bool value)
{
    // Cache "true" and "false" so we dont 
    // create too many unnecessary nodes
    static const JsonBasicNode<JNT_BOOL, bool> TRUE_NODE(true);
    static const JsonBasicNode<JNT_BOOL, bool> FALSE_NODE(false);
    static JsonNodePtr TRUE_NODE_PTR(&TRUE_NODE);
    static JsonNodePtr FALSE_NODE_PTR(&FALSE_NODE);

    return value ? TRUE_NODE_PTR : FALSE_NODE_PTR;
}

//*****************************************************************************
/*!
 *  \brief  Create an integer node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::IntNode(int value)
{
    return new JsonBasicNode<JNT_INT, int>(value);
}

//*****************************************************************************
/*!
 *  \brief  Create a double node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::DoubleNode(double value)
{
    return new JsonBasicNode<JNT_DOUBLE, double>(value);
}

//*****************************************************************************
/*!
 *  \brief  Create a string node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::StringNode(const std::string &value)
{
    return new JsonBasicNode<JNT_STRING, std::string>(value);
}

//*****************************************************************************
/*!
 *  \brief  Create a list node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::ListNode()
{
    return new JsonListNode();
}

//*****************************************************************************
/*!
 *  \brief  Create an object node.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
JsonNodeFactory::ObjectNode()
{
    return new JsonObjectNode();
}

//*****************************************************************************
/*!
 *  \brief  Format json strings
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
std::ostream &
JsonFormatter::FormatString(std::ostream &out, const std::string &str)
{
    out << '"';
    for (std::string::const_iterator iter = str.begin(); iter != str.end(); ++iter)
    {
        char ch = *iter;
        switch (ch)
        {
            case '"':   out << "\\\""; break;
            case '/':   out << "\\/"; break;
            case '\\':  out << "\\\\"; break;
            case '\b':  out << "\\b"; break;
            case '\f':  out << "\\f"; break;
            case '\n':  out << "\\n"; break;
            case '\r':  out << "\\r"; break;
            case '\v':  out << "\\v"; break;
            case '\t':  out << "\\t"; break;
            default:    out << ch; break;
        }
    }
    out << '"';
    return out;
}

//*****************************************************************************
/*!
 *  \brief  Format a json node to an output stream
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
std::ostream &
DefaultJsonFormatter::Format(std::ostream &out, const JsonNodePtr &node)
{
    return FormatExt(out, node, false) << std::endl;
}

//*****************************************************************************
/*!
 *  \brief  Format a json node to an output stream
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
std::ostream &
DefaultJsonFormatter::FormatExt(std::ostream &out, const JsonNodePtr &node, bool appendNewline)
{
    if (!node)
    {
        out << "null";

        if (appendNewline)
            out << std::endl;

        return out;
    }

    switch (node->Type())
    {
        case JNT_NULL: out << "null"; break;
        case JNT_BOOL: out << (node->Value<bool>() ? "true" : "false"); break;
        case JNT_INT: out << node->Value<int>(); break;
        case JNT_DOUBLE: out << node->Value<double>(); break;
        case JNT_STRING: JsonFormatter::FormatString(out, node->Value<std::string>()); break;
        case JNT_LIST:
            out << "[ ";
            for (unsigned i = 0, size = node->Size();i < size;++i)
            {
                if (i > 0)
                    out << ", ";

                FormatExt(out, node->Get(i), appendNewline);
            }
            out << " ]";
            break ;
        case JNT_OBJECT: 
            out << "{ ";
            
            JsonObjectNode *objNode = static_cast<JsonObjectNode *>(node.Data());

            JsonObjectNode::iterator iter = objNode->begin();
            unsigned count = 0;

            for (;iter != objNode->end(); ++iter, ++count)
            {
                if (count != 0)
                    out << ", ";
                JsonFormatter::FormatString(out, iter->first);
                out << ": ";
                FormatExt(out, iter->second, appendNewline);
            }
            out << " }";
            break ;
    }

    if (appendNewline)
        out << std::endl;

    return out;
}

const char AMFFormatter::MARKER_UNDEF        =   0x00;
const char AMFFormatter::MARKER_NULL         =   0x01;
const char AMFFormatter::MARKER_FALSE        =   0x02;
const char AMFFormatter::MARKER_TRUE         =   0x03;
const char AMFFormatter::MARKER_INT          =   0x04;
const char AMFFormatter::MARKER_DOUBLE       =   0x05;
const char AMFFormatter::MARKER_STRING       =   0x06;
const char AMFFormatter::MARKER_XMLDOC       =   0x07;
const char AMFFormatter::MARKER_DATE         =   0x08;
const char AMFFormatter::MARKER_ARRAY        =   0x09;
const char AMFFormatter::MARKER_OBJECT       =   0x0A;
const char AMFFormatter::MARKER_XML          =   0x0B;
const char AMFFormatter::MARKER_BYTEARRAY    =   0x0C;

//*****************************************************************************
/*!
 *  \brief  Writes the JSON as AMF payload to a memory buffer.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void
AMFFormatter::Format(SMemBuff *pMem, const JsonNodePtr &node)
{
    if (!node)
    {
        pMem->Write(&MARKER_NULL, 1);
        return ;
    }

    double d;
    unsigned char intBytes[4];
    int  length;

    switch (node->Type())
    {
        case JNT_NULL:
            pMem->Write(&MARKER_NULL, 1);
            break;
        case JNT_BOOL:
            pMem->Write((node->Value<bool>() ? &MARKER_TRUE : &MARKER_FALSE), 1);
            break;
        case JNT_INT:
            pMem->Write(&MARKER_INT, 1); 
            length = encodeInt(node->Value<int>(), intBytes);
            pMem->Write(intBytes, length);
            break ;
        case JNT_DOUBLE:
            d = node->Value<double>();
            pMem->Write(&MARKER_DOUBLE, 1); 
            pMem->Write(((char *)&d), 8);
            break ;
        case JNT_STRING: 
            pMem->Write(&MARKER_STRING, 1); 
            encodeUTF8(node->Value<std::string>(), pMem);
            break;
        case JNT_LIST:
        {
            pMem->Write(&MARKER_ARRAY, 1); 
            unsigned size = node->Size();
            length = encodeInt(((size << 1) | 1), intBytes);    // array size
            pMem->Write(intBytes, length);
            encodeUTF8("", pMem);                               // associative part of the array
            for (unsigned i = 0;i < size;++i)
            {
                Format(pMem, node->Get(i));
            }
        } break ;
        case JNT_OBJECT: 
        {
            pMem->Write(&MARKER_ARRAY, 1); 

            // write size of dense portion of the array - 0
            // length = encodeInt(1, intBytes);    // array size
            // pMem->Write(intBytes, length);
            pMem->Write("\001", 1);

            JsonObjectNode *objNode = static_cast<JsonObjectNode *>(node.Data());
            JsonObjectNode::iterator iter = objNode->begin();
            for (;iter != objNode->end(); ++iter)
            {
                // attrib name
                encodeUTF8(iter->first, pMem);
                // attrib value
                Format(pMem, iter->second);
            }
            encodeUTF8("", pMem);       // terminating blank string
        } break ;
    }
}

//*****************************************************************************
/*!
 *  \brief  Format a json node to an output stream in AMF.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
std::ostream &
AMFFormatter::Format(std::ostream &out, const JsonNodePtr &node)
{
    SMemBuff membuff;

    Format(&membuff, node);

    int len = membuff.Length();

    char bytes[4] = {
        (len >> 24) & 0xff,
        (len >> 16) & 0xff,
        (len >> 8) & 0xff,
        (len) & 0xff,
    };
    out.write(bytes, 4);
    out.write(membuff.Data(), len);
    return out;
}

//*****************************************************************************
/*!
 *  \brief  UTF8 Encodes a string.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
void
AMFFormatter::encodeUTF8(const std::string &input, SMemBuff *pMem)
{
    const static unsigned IO    =   1  << 7;
    const static unsigned IIO   =   3  << 6;
    const static unsigned IIIO  =   7  << 5;
    const static unsigned IIIIO =   15 << 4;
    // const static unsigned I7    =   (1 << 7) - 1;
    const static unsigned I6    =   (1 << 6) - 1;
    const static unsigned I5    =   (1 << 5) - 1;
    const static unsigned I4    =   (1 << 4) - 1;
    const static unsigned I3    =   (1 << 3) - 1;

    unsigned size = input.size();
    if (size == 0)
    {
        // encodeInt(out, 1);
        pMem->Write("\001", 1);
    }
    else
    {
        int strLen = 0;

        // Treat each character as 16 bit unsigned int
        for (unsigned i = 0;i < size; ++i)
        {
            unsigned ch = (input[i] & 0x0000ffff);
            if (ch <= 0x7F)
            {
                strLen++;
            }
            else if (ch <= 0x07FF)
            {
                strLen += 2;
            }
            else if (ch <= 0xFFFF)
            {
                strLen += 3;
            }
            else if (ch <= 0x10FFFF)
            {
                strLen += 4;
            }
            else
            {
                std::cerr << "Error in String: '" << input << "', i: " << i << ", Character:  " << ch << std::endl;
                std::cerr << "----------------------------------------------------------------" << ch << std::endl;
                assert("Invalid character" && false);
            }
        }

        unsigned char bytes[4];
        int intLen = encodeInt((strLen << 1) | 1, bytes);
        pMem->Write(bytes, intLen);
        
        // now go through the string and encode it all
        for (unsigned i = 0;i < size; ++i)
        {
            unsigned ch = input[i];
            if (ch <= 0x7F)
            {
                strLen++;
                bytes[0] = ch & 0xff;
                // out.write(bytes, 1);
                pMem->Write(bytes, 1);
            }
            else if (ch <= 0x07FF)
            {
                strLen += 2;
                bytes[0] = IIO | ((ch >> 6) & I5);
                bytes[1] = IO | (ch & I6);
                // out.write(bytes, 2);
                pMem->Write(bytes, 2);
            }
            else if (ch <= 0xFFFF)
            {
                strLen += 3;
                bytes[0] = IIIO | ((ch >> 12) & I4);
                bytes[1] = IO | ((ch >> 6) & I6);
                bytes[2] = IO | (ch & I6);
                // out.write(bytes, 3);
                pMem->Write(bytes, 3);
            }
            else if (ch <= 0x10FFFF)
            {
                strLen += 4;
                bytes[0] = IIIIO | ((ch >> 18) & I3);
                bytes[1] = IO | ((ch >> 12) & I6);
                bytes[2] = IO | ((ch >> 6) & I6);
                bytes[3] = IO | (ch & I6);
                // out.write(bytes, 4);
                pMem->Write(bytes, 4);
            }
        }
    }
}

//*****************************************************************************
/*!
 *  \brief  AMF Encodes an integer
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
int
AMFFormatter::encodeInt(int input, unsigned char *bytes)
{
    int len = 0;
    int temp[8];
    if ((input & 0xff000000) == 0)
    {
        for (int i = 3;i >= 0;i--)
        {
            temp[len++] = ((input >> (7 * i)) & 0x7F);
        }
    }
    else
    {
        for (int i = 2;i >= 0;i--)
        {
            temp[len++] = ((input >> (8 + 7 * i)) & 0x7F);
        }
        temp[len++] = (input & 0xff);
    }

    int l = 0;
    for (int i = 0;i < len - 1;i++)
    {
        if (temp[i] > 0)
        {
            bytes[l++] = temp[i] | 0x80;
        }
    }
    bytes[l++] = temp[len - 1];
    return l;
}

//====================      Json Builder    =======================//

//*****************************************************************************
/*!
 *  \brief  Build the json tree out of the input stream.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
DefaultJsonBuilder::Build(JsonInputStream *pStream)
{
    JsonNodePtr ReadNode(JsonTokenizer &tokenizer, JsonTokenizer::JsonToken *laToken = NULL);

    JsonTokenizer tokenizer;
    tokenizer.Begin(pStream);

    return ReadNode(tokenizer);
}

//*****************************************************************************
/*!
 *  \brief  Builds a json node recursively.
 *
 *  \version
 *      - Sri Panyam     10/02/2009
 *        Created.
 *
 *****************************************************************************/
JsonNodePtr
ReadNode(JsonTokenizer &tokenizer, JsonTokenizer::JsonToken *laToken = NULL)
{
    JsonTokenizer::JsonToken tok = laToken ? *laToken : tokenizer.NextToken();

    if (tok == JsonTokenizer::JT_INT)
    {
        return JsonNodeFactory::IntNode(atoi(tokenizer.TokenText().c_str()));
    }
    else if (tok == JsonTokenizer::JT_DOUBLE)
    {
        return JsonNodeFactory::DoubleNode(atof(tokenizer.TokenText().c_str()));
    }
    else if (tok == JsonTokenizer::JT_TRUE)
    {
        return JsonNodeFactory::BoolNode(true);
    }
    else if (tok == JsonTokenizer::JT_FALSE)
    {
        return JsonNodeFactory::BoolNode(false);
    }
    else if (tok == JsonTokenizer::JT_STRING)
    {
        return JsonNodeFactory::StringNode(tokenizer.TokenText());
    }
    else if (tok == JsonTokenizer::JT_OPEN_BRACE)
    {
        JsonNodePtr structNode = JsonNodeFactory::ObjectNode();

        tok = tokenizer.NextToken();

        while (tok == JsonTokenizer::JT_IDENT || tok == JsonTokenizer::JT_STRING)
        {
            std::string attrName = tokenizer.TokenText();

            tok = tokenizer.NextToken();

            if (tok != JsonTokenizer::JT_COLON)
            {
                std::cerr << "Expected ':', found " << tok << " instead." << std::endl;
                return JsonNodePtr();
            }

            JsonNodePtr valNode = ReadNode(tokenizer);

            if (!valNode.Data())
            {
                std::cerr << "Not adding value for '" << attrName << "'" << std::endl;
            }
            else
            {
                structNode->Set(attrName, valNode);
            }

            tok = tokenizer.NextToken();

            if (tok == JsonTokenizer::JT_COMA)
            {
                tok = tokenizer.NextToken();
            }
        }

        if (tok != JsonTokenizer::JT_CLOSE_BRACE)
        {
            std::cerr << "Expected close brace, found " << tok << " instead." << std::endl;
            return JsonNodePtr();
        }

        return structNode;
    }
    else if (tok == JsonTokenizer::JT_OPEN_SQUARE)
    {
        JsonNodePtr listNode = JsonNodeFactory::ListNode();

        tok = tokenizer.NextToken();

        while (tok != JsonTokenizer::JT_CLOSE_SQUARE)
        {
            JsonNodePtr child = ReadNode(tokenizer, &tok);
            if ( ! child.Data())
            {
                return JsonNodePtr();
            }

            listNode->Add(child);
            tok = tokenizer.NextToken();

            if (tok == JsonTokenizer::JT_COMA)
            {
                tok = tokenizer.NextToken();
            }
        }

        return listNode;
    }

    return JsonNodePtr();
}
