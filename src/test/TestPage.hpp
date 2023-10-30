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

#pragma once

#include "lv2c/Lv2cElement.hpp"
#include "lv2c/Lv2cTheme.hpp"
#include <string>

using namespace lv2c;

namespace lv2c
{
    class TestPage
    {
    public:
        TestPage(const std::string &title) : title(title) {}
        virtual ~TestPage() {}

        virtual Lv2cTheme::ptr GetTheme()
        {
            return Lv2cTheme::Create(true);
        }

        const std::string &Title() { return title; }

        virtual Lv2cElement::ptr CreatePageView(Lv2cTheme::ptr theme) = 0;

    private:
        std::string title;
    };

} // namespace