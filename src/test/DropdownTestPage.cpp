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

#include "DropdownTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"

using namespace lvtk;

LvtkElement::ptr DropdownTestPage::CreatePageView(LvtkTheme::ptr theme)
{
    auto main = LvtkFlexGridElement::Create();
    main->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        .FlexWrap(LvtkFlexWrap::Wrap)
        .FlexDirection(LvtkFlexDirection::Column)
        .Background(theme->paper)
        .Padding({24, 16, 24, 16});
    {
        auto element = LvtkDropdownElement::Create();
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
