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

#include "DropdownTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"

using namespace lvtk;

Lv2cElement::ptr DropdownTestPage::CreatePageView(Lv2cTheme::ptr theme)
{
    auto main = Lv2cFlexGridElement::Create();
    main->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .FlexDirection(Lv2cFlexDirection::Column)
        .Background(theme->paper)
        .Padding({24, 16, 24, 16});
    {
        auto element = Lv2cDropdownElement::Create();
        main->AddChild(element);
        element->UnselectedText("Test dropdown");
        element->Style().Margin({4, 4, 4, 4}).Width(120);
        element->DropdownItems({
            {0,"Baxandall"},
            {1,"Fender Bassman"},
            {2,"JCM 8000"}
        });
    }
    return main;
}
