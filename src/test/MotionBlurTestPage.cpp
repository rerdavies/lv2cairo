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

#include "MotionBlurTestPage.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkMotionBlurElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkSlideInOutAnimationElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"

using namespace lvtk;

static LvtkElement::ptr Frame(LvtkElement::ptr element)
{
    auto container = LvtkContainerElement::Create();
    container->Style()
        .BorderColor(LvtkColor("#808080"))
        .BorderWidth({1});
    container->AddChild(element);
    return container;
}

LvtkElement::ptr MotionBlurTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    auto main = LvtkFlexGridElement::Create();
    main->Style()
        .Background(theme->paper)
        .Padding({24, 16, 24, 16})
        .VerticalAlignment(LvtkAlignment::Stretch)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::Wrap)
        .FlexColumnGap(16)
        .FlexRowGap(16);


    LvtkSlideInOutAnimationElement::ptr slide;
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .BorderWidth({1})
            .BorderColor(theme->secondaryTextColor);

        {
            slide = LvtkSlideInOutAnimationElement::Create();

            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(300)
                    .Height(300)
                    .FontSize(LvtkMeasurement::Point(22))
                    .Padding(30)
                    .SingleLine(false)
                    .Background(LvtkColor{"#000000"});
                ;
                slide->AddChild(typography);
            }

            container->AddChild(slide);
        }
        main->AddChild(container);
    }

    {
        auto dropdown = LvtkDropdownElement::Create();
        #define SLIDE_ITEM(x) \
            LvtkDropdownItem {\
                (selection_id_t)(int)LvtkSlideAnimationType::x,\
                #x\
            },

        std::vector<LvtkDropdownItem> items {
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
                slide->StartAnimation((LvtkSlideAnimationType)selection,150);
            }
        );
        main->AddChild(dropdown);
    }

    {
        // divider.
        auto element = LvtkElement::Create();
        element->Style()
            .Width(LvtkMeasurement::Percent(100))
            .Height(1)
            .Background(LvtkColor(0.5,0.5,0.5,0.1));
        main->AddChild(element);
    }

    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Background(LvtkColor("#FFFFFF"))
            .Padding({4});
        {
            auto blur = LvtkMotionBlurElement::Create();
            blur->Style()
                .Background(theme->background);
            {
                auto element = LvtkMotionBlurElement::Create();
                {
                    auto typography = LvtkTypographyElement::Create();
                    typography->Text("The quick brown fox jumped over the lazy dog.");
                    typography->Variant(LvtkTypographyVariant::BodySecondary);
                    typography->Style()
                        .Width(100)
                        .SingleLine(false)
                        .Background(LvtkColor{"#000000"});
                    ;
                    element->AddChild(typography);
                }
                blur->AddChild(element);
            }
            blur->Blur(LvtkPoint(0, 0), LvtkPoint(0, 0));
            container->AddChild(Frame(blur));
        }
        main->AddChild(container);
    }
     {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Background(LvtkColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = LvtkMotionBlurElement::Create();

            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Background(LvtkColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(LvtkPoint(-23, 0), LvtkPoint(-18, 0));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Background(LvtkColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = LvtkMotionBlurElement::Create();

            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Color(LvtkColor("#FF0000"))
                    .Background(LvtkColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(LvtkPoint(0, -23), LvtkPoint(0, -18));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Background(LvtkColor("#FFFFFF"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);
        {
            auto element = LvtkMotionBlurElement::Create();

            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Background(LvtkColor("#000000"));
                ;
                element->AddChild(typography);
            }
            element->Blur(LvtkPoint(0, 23), LvtkPoint(0, 18));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto container = LvtkContainerElement::Create();
        container->Style()
            .Background(LvtkColor("#000000"))
            .BorderColor(theme->secondaryTextColor)
            .Padding(4);


        {
            auto element = LvtkMotionBlurElement::Create();

            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("The quick brown fox jumped over the lazy dog.");
                typography->Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .Width(100)
                    .SingleLine(false)
                    .Color(LvtkColor("#202080"))
                    .Background(LvtkColor("#E0E0E0"));
                ;
                element->AddChild(typography);
            }
            element->Blur(LvtkPoint(20, 0), LvtkPoint(24, 0));
            container->AddChild(element);
        }
        main->AddChild(Frame(container));
    }
    {
        auto typography = LvtkTypographyElement::Create();
        typography->Text("The quick brown fox jumped over the lazy dog.");
        typography->Variant(LvtkTypographyVariant::BodySecondary);
        typography->Style()
            .Width(100)
            .SingleLine(false)
            .Color(LvtkColor("#602020"))
            .Background(LvtkColor("#E0E0E0"));
        ;
        main->AddChild(Frame(typography));
    }
    return (main);
}
