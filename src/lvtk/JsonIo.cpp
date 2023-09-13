// Copyright (c) 2022 Robin Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "lvtk/JsonIo.hpp"

#include "lvtk/IcuString.hpp"
#include "Utf8Utils.hpp"

#include <sstream>
#include <cmath>

using namespace lvtk;


static constexpr uint16_t UTF16_SURROGATE_1_BASE = 0xD800U;
static constexpr uint16_t UTF16_SURROGATE_2_BASE = 0xDC00U;
static constexpr uint16_t UTF16_SURROGATE_MASK = 0x3FFU;

void json_reader::skip_whitespace()
{
    char c;
    while (true)
    {
        int ic = s.peek();
        if (ic == -1)
            break;
        if (is_whitespace((char)ic))
        {
            c = get();
        }
        else if (ic == '/')
        {
            get();
            int c2 = s.peek();
            if (c2 == '/')
            {
                // skip to end of line.
                get();
                while (true)
                {
                    c2 = s.peek();
                    if (c2 == '\r' || c2 == '\n')
                    {
                        get(); // and continue.
                        break;
                    }
                    if (c2 == -1)
                    {
                        break;
                    }
                    get();
                }
            }
            else if (c2 == '*')
            {
                get();
                int level = 1;
                while (true)
                {
                    c = get();
                    if (c == '*' && s.peek() == '/')
                    {
                        get();
                        if (--level == 0)
                        {
                            break;
                        }
                    }
                    if (c == '/' && s.peek() == '*')
                    {
                        get();
                        ++level;
                    }
                }
            }
            else
            {

                throw_format_error();
            }
        }
        else
        {

            break;
        }
    }
}

std::string json_reader::read_string()
{
    // To completely normalize UTF-32 values we must covert to UTF-16, resolve surrogate pairs, and then convert UTF-32 to UTF-8.

    skip_whitespace();
    char c;
    char startingCharacter;
    startingCharacter = get();
    if (startingCharacter != '\'' && startingCharacter != '\"')
    {
        throw_format_error();
    }
    std::stringstream s;

    while (true)
    {
        c = get();
        if (c == startingCharacter)
        {
            if (s.peek() == startingCharacter) //  "" -> "
            {
                get();
                s.put(c);
            }
            else
            {
                break;
            }
        }
        if (c != '\\')
        {
            s << (char)c;
        }
        else
        {
            c = get();
            switch (c)
            {
            case '"':
            case '\\':
            default:
                s << c;
                break;
            case 'r':
                s << '\r';
                break;
            case 'b':
                s << '\b';
                break;
            case 'f':
                s << '\f';
                break;
            case 'n':
                s << '\n';
                break;
            case 't':
                s << '\t';
                break;
            case 'u':
            {
                char16_t uc = read_u_escape();
                std::basic_stringstream<char16_t> ss16;
                if (uc >= UTF16_SURROGATE_1_BASE && uc <= UTF16_SURROGATE_1_BASE + UTF16_SURROGATE_MASK)
                {
                    // MUST be a UTF16_SURROGATE 2 to be legal.
                    c = get();
                    if (c != '\\')
                        throw_format_error("Invalid UTF16 surrogate pair");
                    c = get();
                    if (c != 'u')
                        throw_format_error("Invalid UTF16 surrogate pair");
                    char16_t uc2 = read_u_escape();
                    if (uc2 < UTF16_SURROGATE_2_BASE || uc2 > UTF16_SURROGATE_2_BASE + UTF16_SURROGATE_MASK)
                    {
                        throw_format_error("Invalid UTF16 surrogate pair");
                    }
                    ss16 << uc << uc2;
                }
                else
                {
                    ss16 << uc;
                }
                s << Utf16ToUtf8(ss16.str());
            }
            break;
            }
        }
    }
    return s.str();
}
uint16_t json_reader::read_hex()
{
    char c;
    c = get();
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    throw_format_error("Invalid \\u escape character");
    return 0;
}

char16_t json_reader::read_u_escape()
{
    int result = 0;
    for (int i = 0; i < 4; ++i)
    {
        result = result*16+read_hex();
    }
    return (char16_t)result;
}

bool json_reader::is_complete()
{
    skip_whitespace();
    return s.peek() == -1;
}

void json_reader::consume_token(const char *expectedToken, const char *errorMessage)
{
    skip_whitespace();

    const char *p = expectedToken;
    while (*p != '\0')
    {
        char expectedChar = *p++;
        int c = s.get();
        if (expectedChar != c)
        {
            this->throw_format_error(errorMessage);
        }
    }
}

void json_reader::consume(char expected)
{
    skip_whitespace();
    char c;
    c = get();
    if (c != expected)
    {
        std::stringstream s;
        s << "Expecting '" << expected << "'";
        throw_format_error(s.str().c_str());
    }
}

void json_reader::consume(const char *str)
{
    skip_whitespace();
    const char *p = str;
    while (*p != 0)
    {
        char c = get();
        if (c != *p)
        {
            std::stringstream s;
            s << "Expecting '" << str << "'";
            throw_format_error(s.str());
        }
    }
}

void json_reader::throw_format_error()
{
    throw json_exception("Invalid file format");
}

void json_reader::throw_format_error(const std::string &message)
{
    std::stringstream s;
    s << "Invalid file format. " << message;
    throw json_exception(s.str());
}

void json_reader::read(bool *value)
{
    if (peek() == 't')
    {
        consume_token("true","Expecting a value.");
        *value = true;
    }
    else if (peek() == 'f')
    {
        consume_token("false","Expecting a value.");
        *value = false;
    }
    else
    {
        std::stringstream s;
        s << "Unexpected character: '" << (char)peek() << '\'';
        throw_format_error(s.str());
    }
}

void json_writer::write(bool value)
{
    check_indent();
    check_space();
    s << (value ? "true" : "false");
}
void json_writer::write_null()
{
    check_indent();
    check_space();
    s << "null";
    needs_space_ = true;
}

void json_writer::check_indent()
{
    if (compressed)
        return;
    if (new_line)
    {
        new_line = false;
        for (int64_t i = 0; i < indent_count; ++i)
        {
            s << ' ';
        }
        needs_space_ = false;
    }
}
void json_writer::check_space()
{
    if (compressed)
        return;

    if (needs_space_)
    {
        needs_space_ = false;
        s << ' ';
    }
}

void json_writer::indent()
{
    indent_count += 4;
}
void json_writer::unindent()
{
    indent_count -= 4;
    if (indent_count < 0)
    {
        throw json_exception("Mismatched unindent() call.");
    }
}

void json_writer::endl()
{
    if (compressed)
        return;
    s << std::endl;
    new_line = true;
    needs_space_ = false;
}

void json_writer::write_raw(const char *text)
{
    check_indent();
    check_space();
    s << text;
    needs_space_ = false;
}

void json_writer::write(const std::string &value)
{
    check_indent();
    check_space();

    std::u16string s16 = Utf8ToUtf16(value);
    s << '"';
    for (char16_t c : s16)
    {
        switch (c)
        {
        case '\r':
            s << "\\r";
            break;
        case '\n':
            s << "\\n";
            break;
        case '"':
            s << "\\\"";
            break;
        case '\\':
            s << "\\\\";
            break;
        case '\t':
            s << "\\t";
            break;
        default:
        {
            if (c >= 10 && c < 0x7F) // exclude 7F=del.
            {
                s << (char)c;
            }
            else
            {
                s << "\\u";
                for (int shift = 12; shift >= 0; shift -= 4)
                {
                    static char hex[] = "0123456789ABCDEF";
                    s << hex[(c >> shift) & 0x0F];
                }
            }
            break;
        }
        }
    }
    s << '"';

    needs_space_ = true;
}

void json_writer::write(double value)
{
    check_indent();
    check_space();
    if (std::isnan(value))
    {
        if (allow_nan)
        {
            s << "NaN";
            return;
        }
        throw json_exception("Unable to write NaN value.");
    }
    s.precision(precision);
    s << value;
}

void json_writer::set_precision(int precision)
{
    this->precision = precision;
}

void json_writer::set_allow_nan(bool value)
{
    this->allow_nan = value;
}

void json_writer::start_array()
{
    check_indent();
    s << '[';
    endl();
    indent();
}
void json_writer::end_array()
{
    unindent();
    check_indent();
    s << "]";
}
void json_writer::start_object()
{
    check_indent();
    s << '{';
    endl();
    indent();
}
void json_writer::end_object()
{
    unindent();
    check_indent();
    s << "}";
}

json_writer::json_writer(std::ostream &s, bool compressed)
    : s(s), compressed(compressed)
{
    s.exceptions(std::istream::failbit | std::istream::badbit);
}

void json_reader::read(std::string *value)
{
    *value = read_string();
}

void json_reader::read(double *value)
{
    skip_whitespace();
    if (allowNaN_)
    {
        if (peek() == 'N')
        {
            consume_token("NaN", "Expecting a number.");

            *value = std::nan("");
            return;
        }
    }

    s >> *value;
    if (s.fail())
        throw json_exception("Invalid format.");
}

json_reader::json_reader(std::istream &input)
    : s(input)
{
    s.exceptions(std::istream::failbit | std::istream::badbit);
}

void json_reader::read_null()
{
    consume_token("null","Expecting a value.");
}



void json_writer::needs_space(bool value)
{
    this->needs_space_ = value;
}
