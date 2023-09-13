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

#include "DialTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkDialElement.hpp"
#include "lvtk/LvtkPngDialElement.hpp"
#include "lvtk/LvtkPngStripElement.hpp"

#include "lvtk/LvtkVerticalStackElement.hpp"

using namespace lvtk;

static LvtkElement::ptr TitleBox(const std::string &title, LvtkElement::ptr control)
{
    auto stack = LvtkVerticalStackElement::Create();
    stack->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .Margin(8);

    {
        auto element = LvtkTypographyElement::Create();
        element->Text(title).Variant(LvtkTypographyVariant::Caption);
        element->Style().HorizontalAlignment(LvtkAlignment::Center).Margin({0, 0, 0, 8});
        stack->AddChild(element);
    }
    {
        auto element = LvtkContainerElement::Create();
        stack->AddChild(element);
        element->AddChild(control);
        element->Style().HorizontalAlignment(LvtkAlignment::Center);
    }
    return stack;
}

LvtkElement::ptr DialTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkFlexGridElement::ptr main = LvtkFlexGridElement::Create();
    main->Style().FlexWrap(LvtkFlexWrap::Wrap);
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16});
    {
        LvtkTypographyElement::ptr title = LvtkTypographyElement::Create();
        title->Variant(LvtkTypographyVariant::Title).Text("Dial Test");
        title->Style()
            .Padding({4})
            .Margin({0, 0, 0, 16})
            .BorderWidth({0, 0, 0, 1})
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .BorderColor(LvtkColor(1, 1, 1, 0.25));
        main->AddChild(title);
    }
    {
        auto element = LvtkDialElement::Create();
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("DEFAULT", element));
    }
    {
        auto element = LvtkDialElement::Create();
        element->Style().Color(LvtkColor(0.5, 0.5, 1.0));
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("TINTED", element));
    }

    {
        auto element = LvtkPngDialElement::Create();
        element->Source("dial_strip.png");
        element->Style().Margin(8);
        main->AddChild(
            TitleBox("CUSTOM PNG STRIP", element));
    }

    {
        auto element = LvtkPngDialElement::Create();
        element->Source("FenderDial.png");
        element->Style().Margin(8).Width(96).Height(96);
        main->AddChild(
            TitleBox("CUSTOM PNG STRIP", element));
        if (!theme->isDarkTheme)
        {
            // disable hover opacity.
            LvtkHoverOpacity hoverOpacity { 1.0f,1.0f,1.0f,1.0f};
            element->HoverOpacity(hoverOpacity);
        }
    }

    // {
    //     for (double value : std::vector<double>{0, 0.2, 0.9, 1.0})
    //     {
    //         auto element = LvtkPngStripElement::Create();
    //         element->Style().Width(52.0).Opacity(0.75);
    //         element->Source("dial_strip.png");
    //         element->Value(value);
    //         main->AddChild(
    //             TitleBox("CUSTOM PNG STRIP", element));
    //     }
    // }

    return std::static_pointer_cast<LvtkElement>(main);
}
