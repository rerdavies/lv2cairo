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

#include "TypographyTestPage.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"

using namespace lvtk;

Lv2cElement::ptr TypographyTestPage::CreatePageView(Lv2cTheme::ptr theme) 
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({16, 8, 16, 8});

    {
        Lv2cTypographyElement::ptr title = Lv2cTypographyElement::Create();
        title->Variant(Lv2cTypographyVariant::Title).Text("Typography Theme Styles Test");

        title->Style().Padding({0, 8, 0, 8});

        main->AddChild(title);
    }
    {
        Lv2cTypographyElement::ptr heading = Lv2cTypographyElement::Create();
        heading->Variant(Lv2cTypographyVariant::Heading).Text("Heading");
        heading->Style().Padding({0, 8, 0, 8});

        main->AddChild(heading);
    }
    {
        Lv2cTypographyElement::ptr body = Lv2cTypographyElement::Create();
        body->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Body primary");
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);
    }
    {
        Lv2cTypographyElement::ptr body = Lv2cTypographyElement::Create();
        body->Variant(Lv2cTypographyVariant::BodySecondary).Text("Body secondary");
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);
    }
    {
        Lv2cTypographyElement::ptr body = Lv2cTypographyElement::Create();
        body->Variant(Lv2cTypographyVariant::Caption).Text("CAPTION TEXT");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);

    }

    {
        Lv2cTypographyElement::ptr element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Small caps");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontVariant(Lv2cFontVariant::SmallCaps);

        main->AddChild(element);

    }

    {
        Lv2cTypographyElement::ptr element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Italic");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontStyle(Lv2cFontStyle::Italic);

        main->AddChild(element);

    }
    {
        Lv2cTypographyElement::ptr element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Condensed");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontStretch(Lv2cFontStretch::Condensed);

        main->AddChild(element);

    }
    {
        Lv2cTypographyElement::ptr element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Light");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontWeight(Lv2cFontWeight::Light).FontSize(Lv2cMeasurement::Point(14));

        main->AddChild(element);

    }
    {
        Lv2cTypographyElement::ptr element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodyPrimary).Text("Thin");
        // body->Style().BorderColor(Lv2cColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontWeight(Lv2cFontWeight::Thin);

        main->AddChild(element);

    }


    return std::static_pointer_cast<Lv2cElement>(main);
}
