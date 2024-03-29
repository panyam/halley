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
 *  \file   urlutils.cpp
 *
 *  \brief  URL utilities.
 *
 *  \version
 *      - S Panyam      05/03/2009
 *        Created
 *
 *****************************************************************************/

#include "urlutils.h"

const char   URLUtils::CR          = '\r';
const char   URLUtils::LF          = '\n';
const char * URLUtils::CRLF        = "\r\n";
const char * URLUtils::SEPARATORS  = "()<>@,;:\\\"/[]?={} \t";
static const std::string base64_chars   = 
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz"
                         "0123456789+/";

static const std::string reserved_chars = ";/?:@&=+$,";
static const std::string mark_chars = "-_.!~*'()";


//! Tells if a character is a reserved character
bool IsReservedChar(char ch)
{
    return reserved_chars.find(ch) > 0;
}

bool IsUnreservedChar(char ch)
{
    return isalnum(ch) || (mark_chars.find(ch) > 0);
}

std::string URLUtils::Escape(const std::string &str)
{
    return str;
}

char hex2dec(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return 10 + (ch - 'A');
    else if (ch >= 'a' && ch <= 'f')
        return 10 + (ch - 'a');
    return -1;
}

std::string URLUtils::Unescape(const std::string &str)
{
    const char *buff    = str.c_str();
    const char *pStart  = buff;
    const char *pEnd    = pStart + str.size();
    std::stringstream out;

    while (pStart < pEnd)
    {
        const char *pos = strchr(pStart, '%');
        if (pos == NULL)
        {
            out << pStart;
            pStart = pEnd;
        }
        else
        {
            out << std::string(pStart, pos - pStart);

            pStart = pos;
            // skip the '%'
            if (isxdigit(pStart[1]) && isxdigit(pStart[2]))
            {
                char ch = (hex2dec(pStart[1]) * 16) + hex2dec(pStart[2]);
                out << ch;
                pStart += 3;
            }
            else
            {
                out << *pStart;
                pStart++;
            }
        }
    }

    return out.str();
}

// Reads a line till the CRLF
std::string URLUtils::ReadTillCrLf(std::istream &input)
{
    std::stringstream out;
    int ch = input.get();
    while (!input.bad() && !input.eof() && ch != CR && ch != LF)
    {
        out << (char)ch;
    	ch = input.get();
    }
    ch = input.get();
    return out.str();
}


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string URLUtils::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string URLUtils::base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

/**
 * Given a url query string (without the leading ? mark), this allows
 * incremental (url decoded) extraction of each query name and its value.
 * Returns true if a query was extracted, false otherwise.
 * queryString is updated to point to the start of the next query.
 * qName and qValue are updated with the values of the query parameter.
 */
bool URLUtils::ExtractNextQuery(const char *&queryString, std::string &qName, std::string &qValue)
{
    if (queryString == NULL) return false;

    // skip initial spaces and '&'s
    while (*queryString != 0 && (isspace(*queryString) || (*queryString == '&')))
        queryString++;

    if (*queryString == 0)
        return false;

    const char *eqPos = strchr(queryString, '=');
    if (eqPos == NULL)
        return false;

    qName = Unescape(std::string(queryString, eqPos - queryString));
    
    const char *ampPos = strchr(eqPos + 1, '&');
    qValue = Unescape(ampPos == NULL ? eqPos + 1 : std::string(eqPos + 1, ampPos - (eqPos + 1)));

    queryString = ampPos;
    return true;
}

