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

#include "Lv2cFlexGridElement.hpp"
namespace lvtk
{
    class Lv2cHorizontalStackElement : public Lv2cFlexGridElement
    {
    public:
        using self = Lv2cHorizontalStackElement;
        using super = Lv2cFlexGridElement;
        using ptr = std::shared_ptr<self>;
        ptr Create() { return std::make_shared<self>(); }

    protected:
        void OnMount()
        {
            super::OnMount();
            // set appropriate defaults on the flex grid.
            Classes({std::make_shared<Lv2cStyle>(Lv2cStyle()
                            .FlexDirection(Lv2cFlexDirection::Column)
                            .FlexWrap(Lv2cFlexWrap::NoWrap)
                            .VerticalAlignment(Lv2cAlignment::Start)
                            .HorizontalAlignment(Lv2cAlignment::Start))});
        }
    };
}