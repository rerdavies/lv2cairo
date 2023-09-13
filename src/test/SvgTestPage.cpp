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

#include "SvgTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"

using namespace lvtk;

LvtkElement::ptr SvgTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16})
    .VerticalAlignment(LvtkAlignment::Stretch)
    .HorizontalAlignment(LvtkAlignment::Stretch)
    ;

    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexDirection(LvtkFlexDirection::Row)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch);
        {
            for (double size: std::vector<double>{18,24,36,48,96})
            {
                auto svg = LvtkSvgElement::Create();
                svg->Style().Width(size).Height(size).Margin({8}).HorizontalAlignment(LvtkAlignment::Start).VerticalAlignment(LvtkAlignment::Start);
                svg->Source("fx_dial.svg");
                svg->Style().TintColor(LvtkColor("#E0E0E080"));
                flexGrid->AddChild(svg);
            }
        }
    }

    return (main);
}
