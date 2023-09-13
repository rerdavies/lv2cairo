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

#include "DropShadowTestPage.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkDropShadowElement.hpp"
#include "lvtk/LvtkDialElement.hpp"

using namespace lvtk;

LvtkElement::ptr DropShadowTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(LvtkAlignment::Stretch).HorizontalAlignment(LvtkAlignment::Stretch);

    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexRowGap(16)
            .FlexColumnGap(16)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch)

            ;
        {
            auto dropShadow = LvtkDropShadowElement::Create();
            flexGrid->AddChild(dropShadow);
            dropShadow->Variant(LvtkDropShadowVariant::InnerDropShadow)
                .Radius(4)
                .XOffset(0)
                .YOffset(2)
                .ShadowOpacity(0.75)
                .ShadowColor(LvtkColor(0, 0, 0))
                ;
            dropShadow->Style()
                .Opacity(0.75);

            {
                auto dial = LvtkSvgElement::Create();
                dial->Source("fx_dial.svg");
                dial->Style().Width(48).Height(48).TintColor(LvtkColor(0.9,1,1));
                dropShadow->AddChild(dial);
            }
        }

        {
            auto element = LvtkContainerElement::Create();
            element->Style()
                .Background(LvtkColor(1, 1, 1))
                .Padding({8});

            flexGrid->AddChild(element);

            auto dropShadow = LvtkDropShadowElement::Create();
            element->AddChild(dropShadow);
            dropShadow->Radius(6.0).ShadowOpacity(0.5).XOffset(0).YOffset(4);
            {

                auto typography = LvtkTypographyElement::Create();
                dropShadow->AddChild(typography);
                typography->Text("Purity ")
                    .Variant(LvtkTypographyVariant::Title);
                typography->Style()
                    .FontSize(LvtkMeasurement::Point(30))
                    .Color(LvtkColor(1, 1, 1))
                    .FontStyle(LvtkFontStyle::Normal)
                    .FontWeight(LvtkFontWeight::Heavy)
                    .Margin(4);
            }
        }

        {
            auto container = LvtkContainerElement::Create();
            container->Style()
                .Background(LvtkColor(1,1,1))
                .Padding({30});
            {
                auto dropShadow = LvtkDropShadowElement::Create();
                dropShadow->Radius(6).ShadowOpacity(0.5).XOffset(2).YOffset(4);
                dropShadow->Style().Background(LvtkColor(1,1,1));
                {
                    auto element = LvtkElement::Create();
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
            auto container = LvtkContainerElement::Create();
            container->Style()
                .Background(LvtkColor(1,1,1))
                .Padding({30});
            {
                auto dropShadow = LvtkDropShadowElement::Create();
                dropShadow->Radius(5).ShadowOpacity(0.6).XOffset(2).YOffset(3);
                dropShadow->Style().Background(LvtkColor(1,1,1)).RoundCorners({8});
                {
                    auto element = LvtkElement::Create();
                    element->Style()
                        .Background(LvtkColor(1,1,1))
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
            auto dropShadow = LvtkDropShadowElement::Create();
            dropShadow->Style().Margin(12);
            dropShadow->Radius(4).ShadowOpacity(0.75).XOffset(1).YOffset(2);
            flexGrid->AddChild(dropShadow);
            {
                auto typography = LvtkTypographyElement::Create();
                dropShadow->AddChild(typography);
                typography->Text("Truth")
                    .Variant(LvtkTypographyVariant::Title);
                typography->Style()
                    .FontSize(LvtkMeasurement::Point(30))
                    .Color(LvtkColor(1, 1, 1))
                    .FontStyle(LvtkFontStyle::Italic)
                    .FontWeight(LvtkFontWeight::Heavy);
            }
        }
    }

    return (main);
}
