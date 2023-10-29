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

#include "DropShadowTestPage.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"
#include "lv2c/Lv2cDialElement.hpp"

using namespace lvtk;

Lv2cElement::ptr DropShadowTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(Lv2cAlignment::Stretch).HorizontalAlignment(Lv2cAlignment::Stretch);

    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(Lv2cFlexWrap::Wrap)
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexAlignItems(Lv2cAlignment::Center)
            .RowGap(16)
            .ColumnGap(16)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Stretch)

            ;
        {
            auto dropShadow = Lv2cDropShadowElement::Create();
            flexGrid->AddChild(dropShadow);
            dropShadow->Variant(Lv2cDropShadowVariant::InnerDropShadow)
                .Radius(4)
                .XOffset(0)
                .YOffset(2)
                .ShadowOpacity(0.75)
                .ShadowColor(Lv2cColor(0, 0, 0))
                ;
            dropShadow->Style()
                .Opacity(0.75);

            {
                auto dial = Lv2cSvgElement::Create();
                dial->Source("fx_dial.svg");
                dial->Style().Width(48).Height(48).TintColor(Lv2cColor(0.9,1,1));
                dropShadow->AddChild(dial);
            }
        }

        {
            auto element = Lv2cContainerElement::Create();
            element->Style()
                .Background(Lv2cColor(1, 1, 1))
                .Padding({8});

            flexGrid->AddChild(element);

            auto dropShadow = Lv2cDropShadowElement::Create();
            element->AddChild(dropShadow);
            dropShadow->Radius(6.0).ShadowOpacity(0.5).XOffset(0).YOffset(4);
            {

                auto typography = Lv2cTypographyElement::Create();
                dropShadow->AddChild(typography);
                typography->Text("Purity ")
                    .Variant(Lv2cTypographyVariant::Title);
                typography->Style()
                    .FontSize(Lv2cMeasurement::Point(30))
                    .Color(Lv2cColor(1, 1, 1))
                    .FontStyle(Lv2cFontStyle::Normal)
                    .FontWeight(Lv2cFontWeight::Heavy)
                    .Margin(4);
            }
        }

        {
            auto container = Lv2cContainerElement::Create();
            container->Style()
                .Background(Lv2cColor(1,1,1))
                .Padding({30});
            {
                auto dropShadow = Lv2cDropShadowElement::Create();
                dropShadow->Radius(6).ShadowOpacity(0.5).XOffset(2).YOffset(4);
                dropShadow->Style().Background(Lv2cColor(1,1,1));
                {
                    auto element = Lv2cElement::Create();
                    element->Style()
                        .Width({100})
                        .Height({100})
                        ;

                    dropShadow->AddChild(element);
                }
                container->AddChild(dropShadow);

            }
            flexGrid->AddChild(container);
        }
        {
            auto container = Lv2cContainerElement::Create();
            container->Style()
                .Background(Lv2cColor(1,1,1))
                .Padding({30});
            {
                auto dropShadow = Lv2cDropShadowElement::Create();
                dropShadow->Radius(5).ShadowOpacity(0.6).XOffset(2).YOffset(3);
                dropShadow->Style().Background(Lv2cColor(1,1,1)).RoundCorners({8});
                {
                    auto element = Lv2cElement::Create();
                    element->Style()
                        .Background(Lv2cColor(1,1,1))
                        .Width({100})
                        .Height({100})
                        .RoundCorners({8})
                        ;

                    dropShadow->AddChild(element);
                }
                container->AddChild(dropShadow);

            }
            flexGrid->AddChild(container);
        }


        {
            auto dropShadow = Lv2cDropShadowElement::Create();
            dropShadow->Style().Margin(12);
            dropShadow->Radius(4).ShadowOpacity(0.75).XOffset(1).YOffset(2);
            flexGrid->AddChild(dropShadow);
            {
                auto typography = Lv2cTypographyElement::Create();
                dropShadow->AddChild(typography);
                typography->Text("Truth")
                    .Variant(Lv2cTypographyVariant::Title);
                typography->Style()
                    .FontSize(Lv2cMeasurement::Point(30))
                    .Color(Lv2cColor(1, 1, 1))
                    .FontStyle(Lv2cFontStyle::Italic)
                    .FontWeight(Lv2cFontWeight::Heavy);
            }
        }
    }

    return (main);
}
