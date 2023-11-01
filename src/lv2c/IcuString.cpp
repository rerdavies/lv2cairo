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
#include <iostream>

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <sstream>

using namespace lv2c;

IcuString::~IcuString()
{
}

#ifdef __linux__
#include <dlfcn.h>
#endif





IcuString::IcuString()
{
    this->m_locale = std::locale("");
    std::string name = m_locale.name();
    if (name == "")
    {
        m_locale = std::locale("en_US.UTF8");
    }
}


std::u32string IcuString::toUpper(const std::u32string &text)
{
    #ifdef __linux__
    auto& f = std::use_facet<std::ctype<wchar_t>>(m_locale);

    std::basic_stringstream<char32_t> s;
    for (auto c: text)
    {
        s << (char32_t)(f.toupper((wchar_t)c));
    }
    #else 
        static_assert("windows wchar_t is 16 bit instead of 32-bit. Adjust accordingly.");
    #endif

    return s.str();
}

// utility wrapper to adapt locale-bound facets for u32string/wbuffer convert
template<class Facet>
struct icu_deletable_facet : Facet
{
    template<class... Args>
    icu_deletable_facet(Args&&... args) : Facet(std::forward<Args>(args)...) {}
    ~icu_deletable_facet() {}
};


std::u32string IcuString::toUtf32(const std::string &text)
{
    // use deprecated converter for now until C++ N fixes it.
    return std::wstring_convert<
            icu_deletable_facet<std::codecvt<char32_t,char,std::mbstate_t>>, char32_t>{}.from_bytes(text) ;  
}

 

std::string IcuString::toUtf8(const std::u32string &text)
{
    // use deprecated converter for now until C++ N fixes it.

    return std::wstring_convert<
        icu_deletable_facet<std::codecvt<char32_t,char,std::mbstate_t>>,
         char32_t>{}
         .to_bytes(text) ;  

}

std::u16string IcuString::toUtf16(const std::string &text)
{
    // use deprecated converter for now until C++ N fixes it.
    return std::wstring_convert<
            std::codecvt_utf8_utf16<char16_t>, char16_t>{}
            .from_bytes(text) ;  
}
std::string IcuString::toUtf8(const std::u16string &text)
{
    // use deprecated converter for now until C++ N fixes it.

    return std::wstring_convert<
        std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(text);

}
std::string IcuString::toUpper(const std::string &text)
{
    auto wText = toUtf32(text);
    return toUtf8(toUpper(wText));
}

int IcuString::collationCompare(const std::string &v1, const std::string &v2)
{
    auto& f = std::use_facet<std::collate<char>>(m_locale);

    return f.compare(
        v1.c_str(), v1.c_str()+v1.length(),
            v2.c_str(), v2.c_str()+v2.length());
}

int IcuString::collationCompare(const std::u16string &v1, const std::u16string &v2)
{
    auto& f = std::use_facet<std::collate<char16_t>>(m_locale);
    return f.compare(
        v1.c_str(), v1.c_str()+v1.length(),
        v2.c_str(), v2.c_str()+v2.length());

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
