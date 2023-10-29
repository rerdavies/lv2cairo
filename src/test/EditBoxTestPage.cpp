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

#include "EditBoxTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cEditBoxElement.hpp"
#include "lv2c/Lv2cNumericEditBoxElement.hpp"
using namespace lvtk;

Lv2cElement::ptr EditBoxTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(Lv2cAlignment::Stretch).HorizontalAlignment(Lv2cAlignment::Stretch);

    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(Lv2cFlexWrap::Wrap)
            .FlexDirection(Lv2cFlexDirection::Column)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Stretch);
        {
            {
                auto stack = Lv2cVerticalStackElement::Create();
                stack->Style().Margin({4}).Width(180);
                flexGrid->AddChild(stack);
                auto title = Lv2cTypographyElement::Create();
                title->Text("Numeric float").Variant(Lv2cTypographyVariant::Caption);
                title->Style().Margin({4});
                stack->AddChild(title);

                auto element = Lv2cNumericEditBoxElement::Create();
                element->Style().TextAlign(Lv2cTextAlign::Center);
                stack->AddChild(element);
                element->Style().HorizontalAlignment(Lv2cAlignment::Stretch);


                auto valueDisplay = Lv2cTypographyElement::Create();
                valueDisplay->Variant(Lv2cTypographyVariant::Caption);
                valueDisplay->Style().HorizontalAlignment(Lv2cAlignment::End);
                stack->AddChild(valueDisplay);

                element->DisplayValueProperty.Bind(valueDisplay->TextProperty);
            }
            {
                auto stack = Lv2cVerticalStackElement::Create();
                stack->Style().Margin({4}).Width(180);
                flexGrid->AddChild(stack);
                auto title = Lv2cTypographyElement::Create();
                title->Text("Numeric uint8").Variant(Lv2cTypographyVariant::Caption);
                title->Style().Margin({4});
                stack->AddChild(title);

                auto element = Lv2cNumericEditBoxElement::Create();
                element->ValueType(Lv2cValueType::Uint8);
                stack->AddChild(element);
                element->Style().HorizontalAlignment(Lv2cAlignment::Stretch);


                auto valueDisplay = Lv2cTypographyElement::Create();
                valueDisplay->Variant(Lv2cTypographyVariant::Caption);
                valueDisplay->Style().HorizontalAlignment(Lv2cAlignment::End);
                stack->AddChild(valueDisplay);

                element->DisplayValueProperty.Bind(valueDisplay->TextProperty);
            }


            {
                auto element = Lv2cEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("abc").Selection({0, 2});
                element->Style()
                    .Margin({4});
            }
            {
                auto element = Lv2cEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("Error")
                    .ShowError(true)
                    .SelectAll();

                element->Style().Margin({4});
            }
            {
                auto element = Lv2cEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("Centered")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(Lv2cTextAlign::Center)
                    ;
            }
            {
                auto element = Lv2cEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("End-aligned")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(Lv2cTextAlign::End)
                    ;
            }
            {
                auto element = Lv2cEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(Lv2cTextAlign::Start)
                    .FontSize(Lv2cMeasurement::Point(24))
                    .Padding({2,12,2,12})
                    .Width(240)
                    ;
            }
        }
    }

    return (main);
}
