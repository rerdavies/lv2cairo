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

#include "PaletteTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cPngElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cScrollContainerElement.hpp"
#include "lv2c/Lv2cCieColors.hpp"
#include "ss.hpp"

using namespace lv2c;

Lv2cElement::ptr MakePalette(Lv2cTheme::ptr theme,const std::string &color)
{
    Lv2cColor cc{color};

    std::vector<int> steps { 50,100,200,300,400,500,600,700,800,900};

    auto main = Lv2cFlexGridElement::Create();
    main->Style()
        .FlexDirection(Lv2cFlexDirection::Column)
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .Width(220)
        ;

    Lv2cColor text = Lv2cColor(1,1,1);
    Lv2cColor invertedText = Lv2cColor(0,0,0);

    for (auto step: steps)
    {
        Lv2cColor currentColor = cc.PaletteColor(step);
        Lv2cHsvColor hsv { currentColor};
        Lv2cColor textColor;
        Lv2cCieLCh cieCurrentColor {currentColor};

        double textDiff = Lv2cColor::ColorDifference(text,currentColor);
        double invertedTextDiff = Lv2cColor::ColorDifference(invertedText,currentColor);

        if (textDiff > invertedTextDiff)
        {
            textColor = text;
        } else {
            textColor = invertedText;
        }
        auto element = Lv2cFlexGridElement::Create();
        element->Style()
            .Background(currentColor)
            .Color(textColor)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .FlexJustification(Lv2cFlexJustification::SpaceBetween)
            .FlexAlignItems(Lv2cAlignment::Center)
            .Padding({16});
        {
            auto textElement = Lv2cTypographyElement::Create();
            textElement->Variant(Lv2cTypographyVariant::BodyPrimary)
                .Text(SS(step));
            textElement->Style()
                .Color(textColor);
            element->AddChild(textElement);
        }
        {
            auto textElement = Lv2cTypographyElement::Create();

            textElement->Variant(Lv2cTypographyVariant::BodyPrimary)
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

Lv2cElement::ptr PaletteTestPage::CreatePageView(Lv2cTheme::ptr theme)
{


    auto scrollElement = Lv2cScrollContainerElement::Create();
    scrollElement->HorizontalScrollEnabled(true);
    scrollElement->VerticalScrollOffset(true);
    scrollElement->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch);

    {
        Lv2cFlexGridElement::ptr main = Lv2cFlexGridElement::Create();

        main->Style()
            .Background(theme->paper)
            .Padding({24, 16, 24, 16})
            .ColumnGap(16)
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexWrap(Lv2cFlexWrap::NoWrap);
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

