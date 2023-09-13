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

#include "TypographyTestPage.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"

using namespace lvtk;

LvtkElement::ptr TypographyTestPage::CreatePageView(LvtkTheme::ptr theme) 
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({16, 8, 16, 8});

    {
        LvtkTypographyElement::ptr title = LvtkTypographyElement::Create();
        title->Variant(LvtkTypographyVariant::Title).Text("Typography Theme Styles Test");

        title->Style().Padding({0, 8, 0, 8});

        main->AddChild(title);
    }
    {
        LvtkTypographyElement::ptr heading = LvtkTypographyElement::Create();
        heading->Variant(LvtkTypographyVariant::Heading).Text("Heading");
        heading->Style().Padding({0, 8, 0, 8});

        main->AddChild(heading);
    }
    {
        LvtkTypographyElement::ptr body = LvtkTypographyElement::Create();
        body->Variant(LvtkTypographyVariant::BodyPrimary).Text("Body primary");
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);
    }
    {
        LvtkTypographyElement::ptr body = LvtkTypographyElement::Create();
        body->Variant(LvtkTypographyVariant::BodySecondary).Text("Body secondary");
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);
    }
    {
        LvtkTypographyElement::ptr body = LvtkTypographyElement::Create();
        body->Variant(LvtkTypographyVariant::Caption).Text("CAPTION TEXT");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        body->Style().Padding({0, 8, 0, 8});

        main->AddChild(body);

    }

    {
        LvtkTypographyElement::ptr element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodyPrimary).Text("Small caps");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontVariant(LvtkFontVariant::SmallCaps);

        main->AddChild(element);

    }

    {
        LvtkTypographyElement::ptr element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodyPrimary).Text("Italic");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontStyle(LvtkFontStyle::Italic);

        main->AddChild(element);

    }
    {
        LvtkTypographyElement::ptr element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodyPrimary).Text("Condensed");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontStretch(LvtkFontStretch::Condensed);

        main->AddChild(element);

    }
    {
        LvtkTypographyElement::ptr element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodyPrimary).Text("Light");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontWeight(LvtkFontWeight::Light).FontSize(LvtkMeasurement::Point(14));

        main->AddChild(element);

    }
    {
        LvtkTypographyElement::ptr element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodyPrimary).Text("Thin");
        // body->Style().BorderColor(LvtkColor{1, 1, 1, 0.4}).BorderWidth({1});
        element->Style().Padding({0, 8, 0, 8}).FontWeight(LvtkFontWeight::Thin);

        main->AddChild(element);

    }


    return std::static_pointer_cast<LvtkElement>(main);
}
