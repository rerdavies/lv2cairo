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

#include "PngTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cPngElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"

using namespace lvtk;

Lv2cElement::ptr PngTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(Lv2cAlignment::Stretch).HorizontalAlignment(Lv2cAlignment::Stretch);

    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(Lv2cFlexWrap::Wrap)
            .FlexDirection(Lv2cFlexDirection::Row)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Stretch)
            .FlexAlignItems(Lv2cAlignment::Center);
        {
            for (double size : std::vector<double>{100, 20, 18, 24, 36, 48})
            {
                auto png = Lv2cPngElement::Create();
                png->Style()
                    .Width(size)
                    .Height(size)
                    .Margin({8})
                    .HorizontalAlignment(Lv2cAlignment::Start)
                    .VerticalAlignment(Lv2cAlignment::Start);
                png->Source("Lv2C-Logo-white-512.png");
                flexGrid->AddChild(png);
            }
            auto divider = Lv2cElement::Create();
            flexGrid->AddChild(divider);
            divider->Style()
                .Width(Lv2cMeasurement::Percent(100))
                .Background(Lv2cColor("#FFFFFF20"))
                .Height(1);
            double angle = 14;
            for (double size : std::vector<double>{100, 20, 18, 24, 36, 48})
            {
                auto png = Lv2cPngElement::Create();
                png->Style()
                    .Width(size + 32)
                    .Height(size + 32)
                    .Padding({16})
                    .HorizontalAlignment(Lv2cAlignment::Start)
                    .VerticalAlignment(Lv2cAlignment::Start);
                png->Source("Lv2C-Logo-white-512.png")
                    .Rotation(angle);
                angle += 23;
                flexGrid->AddChild(png);
            }

            auto AddFitOptions = [&flexGrid](const std::string label, Lv2cImageAlignment alignment) mutable
            {
                auto divider = Lv2cElement::Create();
                flexGrid->AddChild(divider);
                divider->Style()
                    .Width(Lv2cMeasurement::Percent(100))
                    .Background(Lv2cColor("#FFFFFF20"))
                    .Height(1)
                    .Margin(8);
                auto typography = Lv2cTypographyElement::Create();
                typography->Text(label);
                flexGrid->AddChild(typography);


                auto png = Lv2cPngElement::Create();
                png->Style()
                    .Width(44)
                    .Height(64)
                    .Margin(8)
                    .Padding(4)
                    .Background(Lv2cColor("#F00"))
                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);

                png = Lv2cPngElement::Create();
                png->Style()
                    .Width(64)
                    .Height(34)
                    .Margin(8)
                    .Padding(4)
                    .Background(Lv2cColor("#F00"))

                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);

                png = Lv2cPngElement::Create();
                png->Style()
                    .Width(64)
                    .Height(64)
                    .Margin(8)
                    .Padding(4)
                    .Background(Lv2cColor("#F00"))

                    ;
                png->ImageAlignment(alignment).Source("scale-test.png");
                flexGrid->AddChild(png);
            };

            AddFitOptions("Fit", Lv2cImageAlignment::Fit);
            AddFitOptions("Stretch", Lv2cImageAlignment::Stretch);
            AddFitOptions("Fill", Lv2cImageAlignment::Fill);
        }
    }

    return (main);
}

