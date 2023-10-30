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
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cScrollBarElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cScrollContainerElement.hpp"

using namespace lv2c;

Lv2cElement::ptr Label(Lv2cElement::ptr element, const std::string &label)
{
    auto container = Lv2cFlexGridElement::Create();
    container->Style()
        .FlexDirection(Lv2cFlexDirection::Column)
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .FlexAlignItems(Lv2cAlignment::Center)
        .Width(200);
    {
        container->AddChild(element);
    }
    {
        auto element = Lv2cTypographyElement::Create();
        element->Text(label).Variant(Lv2cTypographyVariant::BodySecondary);
        element->Style()
            .TextAlign(Lv2cTextAlign::Center)
            .SingleLine(false)
            .FontSize(Lv2cMeasurement::Point(9))
            .Padding({4})
            .HorizontalAlignment(Lv2cAlignment::Stretch);
        container->AddChild(element);
    }
    return container;
}

Lv2cElement::ptr IpsemLorem()
{
    auto element = Lv2cTypographyElement::Create();
    element->Variant(Lv2cTypographyVariant::BodySecondary);
    ;
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
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

Lv2cElement::ptr IpsemLoremWide()
{
    auto container = Lv2cFlexGridElement::Create();
    container->Style()
        .FlexDirection(Lv2cFlexDirection::Row)
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .ColumnGap(16)
        .FlexAlignItems(Lv2cAlignment::Start)
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
        auto element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodySecondary);
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
Lv2cElement::ptr IpsemLoremWideAndTall()
{
    auto container = Lv2cFlexGridElement::Create();
    container->Style()
        .FlexDirection(Lv2cFlexDirection::Row)
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .ColumnGap(16)
        .FlexAlignItems(Lv2cAlignment::Start)
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
        auto element = Lv2cTypographyElement::Create();
        element->Variant(Lv2cTypographyVariant::BodySecondary);
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

Lv2cElement::ptr ScrollBarTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    auto main = Lv2cFlexGridElement::Create();
    main->Style()
        .FlexAlignItems(Lv2cAlignment::Start)
        .FlexDirection(Lv2cFlexDirection::Row)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .ColumnGap(16)
        .Background(theme->paper)
        .Padding({24, 16, 24, 16})
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch);
    if (true)
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);
        {
            auto element = Lv2cScrollBarElement::Create();
            element->ScrollOffset(0)
                .WindowSize(200)
                .DocumentSize(1400);
            element->Orientation(Lv2cScrollBarOrientation::Vertical);
            container->AddChild(element);
        }
        main->AddChild(
            Label(container,
                  "Lv2cVerticalScrollbarElement"));
    }
    if (true)
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);
        {
            auto element = Lv2cScrollBarElement::Create();
            element->ScrollOffset(0)
                .WindowSize(200)
                .DocumentSize(1400);
            element->Orientation(Lv2cScrollBarOrientation::Horizontal);
            container->AddChild(element);
        }
        main->AddChild(Label(container, "Lv2cHorizontalScrollBarElement"));
    }

    {
        auto element = Lv2cScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(false)
            .VerticalScrollEnabled(true);
        element->Child(IpsemLorem());
        main->AddChild(
            Label(element, "Lv2cScrollContainerElement\n(Vertical scroll)"));
    }
    {
        auto element = Lv2cScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(true)
            .VerticalScrollEnabled(false);
        element->Child(IpsemLoremWide());
        main->AddChild(
            Label(element, "Lv2cScrollContainerElement\n(Horizontal scroll)"));
    }
    {
        auto element = Lv2cScrollContainerElement::Create();
        element->Style()
            .Width(200)
            .Height(200)
            .BorderWidth(1)
            .BorderColor(theme->secondaryTextColor);

        element->HorizontalScrollEnabled(true)
            .VerticalScrollEnabled(true);
        element->Child(IpsemLoremWideAndTall());
        main->AddChild(
            Label(element, "Lv2cScrollContainerElement\n(both)"));
    }

    return (main);
}
