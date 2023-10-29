// Copyright (c) 2023 Robin E. R. Davies
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
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include "lv2c/IcuString.hpp"
#include "unicode/uclean.h"
#include "unicode/utypes.h"
#include "unicode/uchar.h"
#include "unicode/locid.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "unicode/unistr.h"
#include <unicode/ucol.h>
#include <iostream>

#include <sstream>

using namespace icu;
using namespace lvtk;

IcuString::~IcuString()
{
    if (cnv)
    {
        ucnv_close(cnv);
        cnv = nullptr;
    }
    if (coll)
    {
        ucol_close(coll);
        coll = nullptr;
    }
}

#ifdef __linux__
#include <dlfcn.h>
#endif


IcuString::IcuString()
{

    UErrorCode errorCode = U_ZERO_ERROR;
    u_init(&errorCode);


    if (U_FAILURE(errorCode))
    {
        fprintf(stderr, "Can't initialize ICU Unicode package. %s\n", u_errorName(errorCode));
        exit(EXIT_FAILURE);
    }

    cnv = ucnv_open("UTF-8", &errorCode);

    if (U_FAILURE(errorCode))
    {
        fprintf(stderr, "Can't create ICU Unicode default converter: %s\n", u_errorName(errorCode));
        exit(EXIT_FAILURE);
    }

    ucnv_setFromUCallBack(cnv, UCNV_FROM_U_CALLBACK_ESCAPE, UCNV_ESCAPE_C, nullptr, nullptr, &errorCode);
    if (U_FAILURE(errorCode))
    {
        fprintf(stderr, "Can't create ICU Unicode escape callback. %s\n", u_errorName(errorCode));
        ucnv_close(cnv);
        exit(EXIT_FAILURE);
    }

    coll = ucol_open("en-US", &errorCode);
    if (U_FAILURE(errorCode))
    {
        fprintf(stderr, "Can't create ICU Unicode Collator. %s\n", u_errorName(errorCode));
        ucnv_close(cnv);
        exit(EXIT_FAILURE);
    }
}

std::u16string IcuString::toUpper(const std::u16string &text)
{

    std::basic_stringstream<char16_t> s;

    bool isError = false;
    const char16_t *input = text.data();
    char16_t c;

    for (size_t i = 0; i < text.length() && !isError; /* U16_NEXT post-increments */)
    {
        size_t previous = i;
        U16_NEXT(input, i, INT32_MAX, c); /* without length because NUL-terminated */
        if (c == u'ß')                    // lowercase ss.
        {
            s << u'ẞ'; // uppercase ss.
        }
        else if (i == previous + 2) // emoji?
        {
            s << text[previous];
            s << text[previous + 1];
        }
        else
        {
            if (c == 0)
            {
                break; /* stop at terminating NUL, no need to terminate buffer */
            }
            c = u_toupper(c);
            s << c;
        }
    }
    return s.str();
}

std::u16string IcuString::toUtf16(const std::string &text)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t size = ucnv_toUChars(cnv, nullptr, 0, text.c_str(), text.length(), &errorCode);
    if (U_FAILURE(errorCode) && errorCode != UErrorCode::U_BUFFER_OVERFLOW_ERROR)
    {
        return u"#icu-error";
    }
    errorCode = U_ZERO_ERROR;

    std::u16string result;
    result.resize(size);
    ucnv_toUChars(cnv, result.data(), result.length() + 1, text.c_str(), text.length(), &errorCode);

    if (U_FAILURE(errorCode) || errorCode == U_STRING_NOT_TERMINATED_WARNING)
    {
        return u"#icu-error";
    }
    return result;
}
std::string IcuString::toUtf8(const std::u16string &text)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t size = ucnv_fromUChars(cnv, NULL, 0, text.c_str(), text.length(), &errorCode);
    if (U_FAILURE(errorCode) && errorCode != UErrorCode::U_BUFFER_OVERFLOW_ERROR)
    {
        return "#icu-error";
    }
    errorCode = U_ZERO_ERROR;
    std::string result;
    result.resize(size);
    ucnv_fromUChars(cnv, result.data(), size + 1, text.c_str(), text.length(), &errorCode);
    if (U_FAILURE(errorCode) || errorCode == U_STRING_NOT_TERMINATED_WARNING)
    {
        return "#error";
    }
    return result;
}

std::string IcuString::toUpper(const std::string &text)
{
    return toUtf8(toUpper(toUtf16(text)));
}

int IcuString::collationCompare(const std::string &v1, const std::string &v2)
{
    return collationCompare(toUtf16(v1), toUtf16(v2));
}

int IcuString::collationCompare(const std::u16string &v1, const std::u16string &v2)
{
    return ucol_strcoll(coll, v1.c_str(), v1.length(), v2.c_str(), v2.length());
}

/*static*/
IcuString *
    IcuString::gIcuStringInstance = nullptr;

/* static*/
int64_t IcuString::gIcuStringRefCount = 0;

IcuString::Ptr::~Ptr()
{
    Release();
}

void IcuString::Ptr::AddRef()
{
    if (m_p)
    {
        IcuString::AddRef();
    }
}

void IcuString::Ptr::Release()
{
    if (m_p)
    {
        IcuString::Release();
        m_p = nullptr;
    }
}

IcuString::Ptr IcuString::Instance()
{
    if (!gIcuStringInstance)
    {
        gIcuStringInstance = new IcuString;
    }
    return Ptr(gIcuStringInstance);
}
/*static*/ void IcuString::AddRef()
{
    ++gIcuStringRefCount;
}
/*static*/
void IcuString::Release()
{
    if (--gIcuStringRefCount == 0)
    {
        delete gIcuStringInstance;
        gIcuStringInstance = nullptr;
    }
}
