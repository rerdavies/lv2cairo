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

#include "PngTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkPngElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"

using namespace lvtk;

LvtkElement::ptr PngTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(LvtkAlignment::Stretch).HorizontalAlignment(LvtkAlignment::Stretch);

    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexDirection(LvtkFlexDirection::Row)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .FlexAlignItems(LvtkAlignment::Center);
        {
            for (double size : std::vector<double>{100, 20, 18, 24, 36, 48})
            {
                auto png = LvtkPngElement::Create();
                png->Style()
                    .Width(size)
                    .Height(size)
                    .Margin({8})
                    .HorizontalAlignment(LvtkAlignment::Start)
                    .VerticalAlignment(LvtkAlignment::Start);
                png->Source("Lv2C-Logo-white-512.png");
                flexGrid->AddChild(png);
            }
            auto divider = LvtkElement::Create();
            flexGrid->AddChild(divider);
            divider->Style()
                .Width(LvtkMeasurement::Percent(100))
                .Background(LvtkColor("#FFFFFF20"))
                .Height(1);
            double angle = 14;
            for (double size : std::vector<double>{100, 20, 18, 24, 36, 48})
            {
                auto png = LvtkPngElement::Create();
                png->Style()
                    .Width(size + 32)
                    .Height(size + 32)
                    .Padding({16})
                    .HorizontalAlignment(LvtkAlignment::Start)
                    .VerticalAlignment(LvtkAlignment::Start);
                png->Source("Lv2C-Logo-white-512.png")
                    .Rotation(angle);
                angle += 23;
                flexGrid->AddChild(png);
            }

            auto AddFitOptions = [&flexGrid](const std::string label, LvtkImageAlignment alignment) mutable
            {
                auto divider = LvtkElement::Create();
                flexGrid->AddChild(divider);
                divider->Style()
                    .Width(LvtkMeasurement::Percent(100))
                    .Background(LvtkColor("#FFFFFF20"))
                    .Height(1)
                    .Margin(8);
                auto typography = LvtkTypographyElement::Create();
                typography->Text(label);
                flexGrid->AddChild(typography);


                auto png = LvtkPngElement::Create();
                png->Style()
                    .Width(44)
                    .Height(64)
                    .Margin(8)
                    .Padding(4)
                    .Background(LvtkColor("#F00"))
                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);

                png = LvtkPngElement::Create();
                png->Style()
                    .Width(64)
                    .Height(34)
                    .Margin(8)
                    .Padding(4)
                    .Background(LvtkColor("#F00"))

                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);

                png = LvtkPngElement::Create();
                png->Style()
                    .Width(64)
                    .Height(64)
                    .Margin(8)
                    .Padding(4)
                    .Background(LvtkColor("#F00"))

                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);
            };

            AddFitOptions("Fit", LvtkImageAlignment::Fit);
            AddFitOptions("Stretch", LvtkImageAlignment::Stretch);
            AddFitOptions("Fill", LvtkImageAlignment::Fill);
        }
    }

    return (main);
}

