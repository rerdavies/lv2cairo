// Copyright (c) 2023 Robin Davies
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

#pragma once

#include <string>
#include <memory>

typedef struct UConverter UConverter;
typedef struct UCollator UCollator;

namespace lvtk
{


    /// @brief A wrapper for the UCI unicode library to provate basic UTF services.
    class IcuString
    {
    public:
        IcuString();
        ~IcuString();

        class Ptr {
        
        private:
            friend class IcuString;
            Ptr(IcuString*p) { m_p = p; AddRef(); }
        public:
            Ptr() { m_p = 0; }
            Ptr(Ptr&&other) { m_p = other.m_p; other.m_p = nullptr; }
            Ptr(const Ptr&other) { m_p = other.m_p; IcuString::AddRef();}
            ~Ptr();

            IcuString::Ptr&operator=(Ptr&&other);
            IcuString::Ptr&operator=(const Ptr&other);
            IcuString&operator*() { return *m_p; }
            IcuString*operator->() { return m_p; }
            IcuString *get() { return m_p; }
            operator bool() const { return m_p != nullptr; }
        private: 
            void AddRef();
            void Release();

            IcuString*m_p;
        };

        static IcuString::Ptr Instance();
        
        static void AddRef();
        static void Release();


        std::string toUpper(const std::string&text);

        std::u16string toUpper(const std::u16string&text);
        std::u16string toUtf16(const std::string&text);
        std::string toUtf8(const std::u16string&text);

        /// @brief Compare using current locale's sorting rules.
        /// @param v1 string
        /// @param v2 string 
        /// @return -1 if v1 < v2, 0 if v1 == v2, +1 if v1 > v2.
        int collationCompare(const std::string &v1, const std::string &v2);
        int collationCompare(const std::u16string &v1, const std::u16string &v2);
    private:
        UConverter *cnv = nullptr;
        UCollator *coll = nullptr;

        static IcuString *gIcuStringInstance;
        static int64_t gIcuStringRefCount;
    };


    ///////////////////////////////


    inline IcuString::Ptr&IcuString::Ptr::operator=(Ptr&&other)
    {
        std::swap(m_p,other.m_p);
        return *this;
    }
    inline IcuString::Ptr&IcuString::Ptr::operator=(
        const Ptr&other)
    {
        Release();
        m_p = other.m_p;
        AddRef();
        return *this;    
    }


}
