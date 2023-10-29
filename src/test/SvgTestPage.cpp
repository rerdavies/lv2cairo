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

#include "SvgTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"

using namespace lvtk;

Lv2cElement::ptr SvgTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16})
    .VerticalAlignment(Lv2cAlignment::Stretch)
    .HorizontalAlignment(Lv2cAlignment::Stretch)
    ;

    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(Lv2cFlexWrap::Wrap)
            .FlexDirection(Lv2cFlexDirection::Row)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Stretch);
        {
            for (double size: std::vector<double>{18,24,36,48,96})
            {
                auto svg = Lv2cSvgElement::Create();
                svg->Style().Width(size).Height(size).Margin({8}).HorizontalAlignment(Lv2cAlignment::Start).VerticalAlignment(Lv2cAlignment::Start);
                svg->Source("fx_dial.svg");
                svg->Style().TintColor(Lv2cColor("#E0E0E080"));
                flexGrid->AddChild(svg);
            }
        }
    }

    return (main);
}
