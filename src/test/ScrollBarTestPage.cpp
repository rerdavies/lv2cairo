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

#include "ScrollBarTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkScrollBarElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkScrollContainerElement.hpp"

using namespace lvtk;

LvtkElement::ptr Label(LvtkElement::ptr element, const std::string &label)
{
    auto container = LvtkFlexGridElement::Create();
    container->Style()
        .FlexDirection(LvtkFlexDirection::Column)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .FlexAlignItems(LvtkAlignment::Center)
        .Width(200);
    {
        container->AddChild(element);
    }
    {
        auto element = LvtkTypographyElement::Create();
        element->Text(label).Variant(LvtkTypographyVariant::BodySecondary);
        element->Style()
            .TextAlign(LvtkTextAlign::Center)
            .SingleLine(false)
            .FontSize(LvtkMeasurement::Point(9))
            .Padding({4})
            .HorizontalAlignment(LvtkAlignment::Stretch);
        container->AddChild(element);
    }
    return container;
}

LvtkElement::ptr IpsemLorem()
{
    auto element = LvtkTypographyElement::Create();
    element->Variant(LvtkTypographyVariant::BodySecondary);
    ;
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .SingleLine(false)
        .Padding({24, 16, 24, 24});
    element->Text(
        "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, "
        "totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta "
        "sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia "
        "consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem "
        "ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut "
        "labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem "
        "ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure "
        "reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem "
        "eum fugiat quo voluptas nulla pariatur?");
    return element;
}

LvtkElement::ptr IpsemLoremWide()
{
    auto container = LvtkFlexGridElement::Create();
    container->Style()
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .ColumnGap(16)
        .FlexAlignItems(LvtkAlignment::Start)
        .Padding({16,16,0,16})
        ;

    static std::vector<std::string> lines = 
    {
        "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, ",
        "totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta ",
        "sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia ",
        "consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ",
        "ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut ",
        "labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ",
        "ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure ",
        "reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem ",
        "eum fugiat quo voluptas nulla pariatur?"
    };
    for (auto& line : lines )
    {
        auto element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodySecondary);
        ;
        element->Style()
            .SingleLine(false)
            .Width(180)
            ;

        element->Text(line);
        container->AddChild(element);
    }
    return container;
}
LvtkElement::ptr IpsemLoremWideAndTall()
{
    auto container = LvtkFlexGridElement::Create();
    container->Style()
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .ColumnGap(16)
        .FlexAlignItems(LvtkAlignment::Start)
        .Padding({16,16,0,16})
        ;

    static std::vector<std::string> lines = 
    {
        "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, "
        "totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta "
        "sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia ",
        "consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem "
        "ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut "
        "labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ",
        "ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure "
        "reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem "
        "eum fugiat quo voluptas nulla pariatur?"
    };
    for (auto& line : lines )
    {
        auto element = LvtkTypographyElement::Create();
        element->Variant(LvtkTypographyVariant::BodySecondary);
        ;
        element->Style()
            .SingleLine(false)
            .Width(180)
            ;

        element->Text(line);
        container->AddChild(element);
    }
    return container;
}

LvtkElement::ptr ScrollBarTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    auto main = LvtkFlexGridElement::Create();
    main->Style()
        .FlexAlignItems(LvtkAlignment::Start)
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::Wrap)
        .ColumnGap(16)
        .Background(theme->paper)
        .Padding({24, 16, 24, 16})
        .VerticalAlignment(LvtkAlignment::Stretch)
        .HorizontalAlignment(LvtkAlignment::Stretch);
    if (true)
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);
        {
            auto element = LvtkScrollBarElement::Create();
            element->ScrollOffset(0)
                .WindowSize(200)
                .DocumentSize(1400);
            element->Orientation(LvtkScrollBarOrientation::Vertical);
            container->AddChild(element);
        }
        main->AddChild(
            Label(container,
                  "LvtkVerticalScrollbarElement"));
    }
    if (true)
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);
        {
            auto element = LvtkScrollBarElement::Create();
            element->ScrollOffset(0)
                .WindowSize(200)
                .DocumentSize(1400);
            element->Orientation(LvtkScrollBarOrientation::Horizontal);
            container->AddChild(element);
        }
        main->AddChild(Label(container, "LvtkHorizontalScrollBarElement"));
    }

    {
        auto element = LvtkScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(false)
            .VerticalScrollEnabled(true);
        element->Child(IpsemLorem());
        main->AddChild(
            Label(element, "LvtkScrollContainerElement\n(Vertical scroll)"));
    }
    {
        auto element = LvtkScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(true)
            .VerticalScrollEnabled(false);
        element->Child(IpsemLoremWide());
        main->AddChild(
            Label(element, "LvtkScrollContainerElement\n(Horizontal scroll)"));
    }
    {
        auto element = LvtkScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(true)
            .VerticalScrollEnabled(true);
        element->Child(IpsemLoremWideAndTall());
        main->AddChild(
            Label(element, "LvtkScrollContainerElement\n(both)"));
    }

    return (main);
}
