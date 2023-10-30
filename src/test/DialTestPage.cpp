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

#include "DialTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cDialElement.hpp"
#include "lv2c/Lv2cPngDialElement.hpp"
#include "lv2c/Lv2cPngStripElement.hpp"

#include "lv2c/Lv2cVerticalStackElement.hpp"

using namespace lv2c;

static Lv2cElement::ptr TitleBox(const std::string &title, Lv2cElement::ptr control)
{
    auto stack = Lv2cVerticalStackElement::Create();
    stack->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .Margin(8);

    {
        auto element = Lv2cTypographyElement::Create();
        element->Text(title).Variant(Lv2cTypographyVariant::Caption);
        element->Style().HorizontalAlignment(Lv2cAlignment::Center).Margin({0, 0, 0, 8});
        stack->AddChild(element);
    }
    {
        auto element = Lv2cContainerElement::Create();
        stack->AddChild(element);
        element->AddChild(control);
        element->Style().HorizontalAlignment(Lv2cAlignment::Center);
    }
    return stack;
}

Lv2cElement::ptr DialTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cFlexGridElement::ptr main = Lv2cFlexGridElement::Create();
    main->Style().FlexWrap(Lv2cFlexWrap::Wrap);
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16});
    {
        Lv2cTypographyElement::ptr title = Lv2cTypographyElement::Create();
        title->Variant(Lv2cTypographyVariant::Title).Text("Dial Test");
        title->Style()
            .Padding({4})
            .Margin({0, 0, 0, 16})
            .BorderWidth({0, 0, 0, 1})
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .BorderColor(Lv2cColor(1, 1, 1, 0.25));
        main->AddChild(title);
    }
    {
        auto element = Lv2cDialElement::Create();
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("DEFAULT", element));
    }
    {
        auto element = Lv2cDialElement::Create();
        element->Style().Color(Lv2cColor(0.5, 0.5, 1.0));
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("TINTED", element));
    }

    {
        auto element = Lv2cPngDialElement::Create();
        element->Source("dial_strip.png");
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("CUSTOM PNG STRIP", element));
    }

    {
        auto element = Lv2cPngDialElement::Create();
        element->Source("FenderDial.png");
        element->Style().Margin(8).Width(96).Height(96);
        main->AddChild(
            TitleBox("CUSTOM PNG STRIP", element));
        if (!theme->isDarkTheme)
        {
            // disable hover opacity.
            Lv2cHoverOpacity hoverOpacity { 1.0f,1.0f,1.0f,1.0f};
            element->HoverOpacity(hoverOpacity);
        }
    }

    // {
    //     for (double value : std::vector<double>{0, 0.2, 0.9, 1.0})
    //     {
    //         auto element = Lv2cPngStripElement::Create();
    //         element->Style().Width(52.0).Opacity(0.75);
    //         element->Source("dial_strip.png");
    //         element->Value(value);
    //         main->AddChild(
    //             TitleBox("CUSTOM PNG STRIP", element));
    //     }
    // }

    return std::static_pointer_cast<Lv2cElement>(main);
}
