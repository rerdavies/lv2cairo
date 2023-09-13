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

#include "PaletteTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkPngElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkScrollContainerElement.hpp"
#include "lvtk/LvtkCieColors.hpp"
#include "ss.hpp"

using namespace lvtk;

LvtkElement::ptr MakePalette(LvtkTheme::ptr theme,const std::string &color)
{
    LvtkColor cc{color};

    std::vector<int> steps { 50,100,200,300,400,500,600,700,800,900};

    auto main = LvtkFlexGridElement::Create();
    main->Style()
        .FlexDirection(LvtkFlexDirection::Column)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .Width(220)
        ;

    LvtkColor text = LvtkColor(1,1,1);
    LvtkColor invertedText = LvtkColor(0,0,0);

    for (auto step: steps)
    {
        LvtkColor currentColor = cc.PaletteColor(step);
        LvtkHsvColor hsv { currentColor};
        LvtkColor textColor;
        LvtkCieLCh cieCurrentColor {currentColor};

        double textDiff = LvtkColor::ColorDifference(text,currentColor);
        double invertedTextDiff = LvtkColor::ColorDifference(invertedText,currentColor);

        if (textDiff > invertedTextDiff)
        {
            textColor = text;
        } else {
            textColor = invertedText;
        }
        auto element = LvtkFlexGridElement::Create();
        element->Style()
            .Background(currentColor)
            .Color(textColor)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .FlexJustification(LvtkFlexJustification::SpaceBetween)
            .FlexAlignItems(LvtkAlignment::Center)
            .Padding({16});
        {
            auto textElement = LvtkTypographyElement::Create();
            textElement->Variant(LvtkTypographyVariant::BodyPrimary)
                .Text(SS(step));
            textElement->Style()
                .Color(textColor);
            element->AddChild(textElement);
        }
        {
            auto textElement = LvtkTypographyElement::Create();

            textElement->Variant(LvtkTypographyVariant::BodyPrimary)
            .Text(currentColor.ToWebString());

            textElement->Style()
                .Color(textColor)
                .MarginLeft(16);
            element->AddChild(textElement);
        }
        main->AddChild(element);
    }
    return main;
}

LvtkElement::ptr PaletteTestPage::CreatePageView(LvtkTheme::ptr theme)
{


    auto scrollElement = LvtkScrollContainerElement::Create();
    scrollElement->HorizontalScrollEnabled(true);
    scrollElement->VerticalScrollOffset(true);
    scrollElement->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);

    {
        LvtkFlexGridElement::ptr main = LvtkFlexGridElement::Create();

        main->Style()
            .Background(theme->paper)
            .Padding({24, 16, 24, 16})
            .FlexColumnGap(16)
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap);
        {
            std::vector<std::string> colors{"#F44336", "#405090","#E91E63", "#9C27B0", "#009688"};

            for (auto &color : colors)
            {
                auto element = MakePalette(theme,color);
                main->AddChild(element);
            }
        }
        scrollElement->Child(main);
    }
    return (scrollElement);
}

