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

#include "MotionBlurTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cMotionBlurElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cSlideInOutAnimationElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"

using namespace lv2c;

static Lv2cElement::ptr Frame(Lv2cElement::ptr element)
{
    auto container = Lv2cContainerElement::Create();
    container->Style()
        .BorderColor(Lv2cColor("#808080"))
        .BorderWidth({1});
    container->AddChild(element);
    return container;
}

Lv2cElement::ptr MotionBlurTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    auto main = Lv2cFlexGridElement::Create();
    main->Style()
        .Background(theme->paper)
        .Padding({24, 16, 24, 16})
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .FlexDirection(Lv2cFlexDirection::Row)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .ColumnGap(16)
        .RowGap(16);


    Lv2cSlideInOutAnimationElement::ptr slide;
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .BorderWidth({1})
            .BorderColor(theme->secondaryTextColor);

        {
            slide = Lv2cSlideInOutAnimationElement::Create();

            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(300)
                    .Height(300)
                    .FontSize(Lv2cMeasurement::Point(22))
                    .Padding(30)
                    .SingleLine(false)
                    .Background(Lv2cColor{"#000000"});
                ;
                slide->AddChild(typography);
            }

            container->AddChild(slide);
        }
        main->AddChild(container);
    }

    {
        auto dropdown = Lv2cDropdownElement::Create();
        #define SLIDE_ITEM(x) \
            Lv2cDropdownItem {\
                (selection_id_t)(int)Lv2cSlideAnimationType::x,\
                #x\
            },

        std::vector<Lv2cDropdownItem> items {
            SLIDE_ITEM(None)
            SLIDE_ITEM(SlideInStart)
            SLIDE_ITEM(SlideInEnd)
            SLIDE_ITEM(SlideInTop)
            SLIDE_ITEM(SlideInBottom)
            SLIDE_ITEM(SlideOutStart)
            SLIDE_ITEM(SlideOutEnd)
            SLIDE_ITEM(SlideOutBottom)
            SLIDE_ITEM(SlideOutTop)
        };
        dropdown->DropdownItems(items);

        dropdown->SelectedId(0);
        
        selectSlideAnimationObserverHandle = dropdown->SelectedIdProperty.addObserver(
            [slide=slide.get()](selection_id_t selection)
            {
                slide->StartAnimation((Lv2cSlideAnimationType)selection,150);
            }
        );
        main->AddChild(dropdown);
    }

    {
        // divider.
        auto element = Lv2cElement::Create();
        element->Style()
            .Width(Lv2cMeasurement::Percent(100))
            .Height(1)
            .Background(Lv2cColor(0.5,0.5,0.5,0.1));
        main->AddChild(element);
    }

    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Background(Lv2cColor("#FFFFFF"))
            .Padding({4});
        {
            auto blur = Lv2cMotionBlurElement::Create();
            blur->Style()
                .Background(theme->background);
            {
                auto element = Lv2cMotionBlurElement::Create();
                {
                    auto typography = Lv2cTypographyElement::Create();
                    typography->Text("The quick brown fox jumped over the lazy dog.");
                    typography->Variant(Lv2cTypographyVariant::BodySecondary);
                    typography->Style()
                        .Width(100)
                        .SingleLine(false)
                        .Background(Lv2cColor{"#000000"});
                    ;
                    element->AddChild(typography);
                }
                blur->AddChild(element);
            }
            blur->Blur(Lv2cPoint(0, 0), Lv2cPoint(0, 0));
            container->AddChild(Frame(blur));
        }
        main->AddChild(container);
    }
     {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Background(Lv2cColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = Lv2cMotionBlurElement::Create();

            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Background(Lv2cColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(Lv2cPoint(-23, 0), Lv2cPoint(-18, 0));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Background(Lv2cColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = Lv2cMotionBlurElement::Create();

            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Color(Lv2cColor("#FF0000"))
                    .Background(Lv2cColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(Lv2cPoint(0, -23), Lv2cPoint(0, -18));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Background(Lv2cColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = Lv2cMotionBlurElement::Create();

            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Background(Lv2cColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(Lv2cPoint(0, 23), Lv2cPoint(0, 18));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = Lv2cContainerElement::Create();
        container->Style()
            .Background(Lv2cColor("#000000"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);


        {
            auto element = Lv2cMotionBlurElement::Create();

            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Color(Lv2cColor("#202080"))
                    .Background(Lv2cColor("#E0E0E0"));
                ;
                element->AddChild(typography);
            }
            element->Blur(Lv2cPoint(20, 0), Lv2cPoint(24, 0));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto typography = Lv2cTypographyElement::Create();
        typography->Text("The quick brown fox jumped over the lazy dog.");
        typography->Variant(Lv2cTypographyVariant::BodySecondary);
        typography->Style()
            .Width(100)
            .SingleLine(false)
            .Color(Lv2cColor("#602020"))
            .Background(Lv2cColor("#E0E0E0"));
        ;
        main->AddChild(Frame(typography));
    }
    return (main);
}
