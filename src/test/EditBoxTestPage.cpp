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
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkEditBoxElement.hpp"
#include "lvtk/LvtkNumericEditBoxElement.hpp"
using namespace lvtk;

LvtkElement::ptr EditBoxTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16}).VerticalAlignment(LvtkAlignment::Stretch).HorizontalAlignment(LvtkAlignment::Stretch);

    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexDirection(LvtkFlexDirection::Column)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch);
        {
            {
                auto stack = LvtkVerticalStackElement::Create();
                stack->Style().Margin({4}).Width(180);
                flexGrid->AddChild(stack);
                auto title = LvtkTypographyElement::Create();
                title->Text("Numeric float").Variant(LvtkTypographyVariant::Caption);
                title->Style().Margin({4});
                stack->AddChild(title);

                auto element = LvtkNumericEditBoxElement::Create();
                element->Style().TextAlign(LvtkTextAlign::Center);
                stack->AddChild(element);
                element->Style().HorizontalAlignment(LvtkAlignment::Stretch);


                auto valueDisplay = LvtkTypographyElement::Create();
                valueDisplay->Variant(LvtkTypographyVariant::Caption);
                valueDisplay->Style().HorizontalAlignment(LvtkAlignment::End);
                stack->AddChild(valueDisplay);

                element->DisplayValueProperty.Bind(valueDisplay->TextProperty);
            }
            {
                auto stack = LvtkVerticalStackElement::Create();
                stack->Style().Margin({4}).Width(180);
                flexGrid->AddChild(stack);
                auto title = LvtkTypographyElement::Create();
                title->Text("Numeric uint8").Variant(LvtkTypographyVariant::Caption);
                title->Style().Margin({4});
                stack->AddChild(title);

                auto element = LvtkNumericEditBoxElement::Create();
                element->ValueType(LvtkValueType::Uint8);
                stack->AddChild(element);
                element->Style().HorizontalAlignment(LvtkAlignment::Stretch);


                auto valueDisplay = LvtkTypographyElement::Create();
                valueDisplay->Variant(LvtkTypographyVariant::Caption);
                valueDisplay->Style().HorizontalAlignment(LvtkAlignment::End);
                stack->AddChild(valueDisplay);

                element->DisplayValueProperty.Bind(valueDisplay->TextProperty);
            }


            {
                auto element = LvtkEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("abc").Selection({0, 2});
                element->Style()
                    .Margin({4});
            }
            {
                auto element = LvtkEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("Error")
                    .ShowError(true)
                    .SelectAll();

                element->Style().Margin({4});
            }
            {
                auto element = LvtkEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("Centered")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(LvtkTextAlign::Center)
                    ;
            }
            {
                auto element = LvtkEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("End-aligned")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(LvtkTextAlign::End)
                    ;
            }
            {
                auto element = LvtkEditBoxElement::Create();
                flexGrid->AddChild(element);
                element->Variant(EditBoxVariant::Underline);
                element->Text("")
                    .SelectAll();

                element->Style()
                    .Margin({4})
                    .TextAlign(LvtkTextAlign::Start)
                    .FontSize(LvtkMeasurement::Point(24))
                    .Padding({2,12,2,12})
                    .Width(240)
                    ;
            }
        }
    }

    return (main);
}
