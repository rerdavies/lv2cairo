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

#include <catch2/catch.hpp>
#include <string>
#include <sstream>
#include <locale>
#include "lv2c/IcuString.hpp"
#include <iostream>


using namespace lv2c;


TEST_CASE("Unicode capitalization test", "[capitalization]")
{
    IcuString icu;


    {
        std::u32string testString(U"şġabcABC😀");
        std::u32string result = icu.toUpper(testString);
        std::u32string expected = U"ŞĠABCABC😀";
        REQUIRE(result == expected);
    }

    {
        std::string testString("abc");
        std::string result = icu.toUpper(testString);
        std::string expected = "ABC";
        REQUIRE(result == expected);
    }

    {
        std::string testString("ωşġßabcABC😀");
        std::u32string wTestString = icu.toUtf32(testString);

        std::string result = icu.toUpper(testString);
        std::string expected = "ΩŞĠßABCABC😀";
        REQUIRE(result == expected);
    }
    {
        std::string testString("şġßabcABC😀");
        std::string result = icu.toUpper(testString);
        std::string expected = "ŞĠßABCABC😀";
        REQUIRE(result == expected);
    }

}
