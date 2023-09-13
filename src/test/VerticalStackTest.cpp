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

#include "VerticalStackTest.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"

using namespace lvtk;

LvtkElement::ptr VerticalStackTestPage::CreatePageView(LvtkTheme::ptr theme)
{
    auto result = std::make_shared<LvtkVerticalStackElement>();
    result->Style()
        .Background(LvtkPattern(LvtkColor(1, 0.5, 0.5)))
        .BorderColor(LvtkColor(0, 0, 0, 0.6))
        .BorderWidth(4.0)
        .Margin(4)
        .Padding(4)
        .FontFamily("Roboto,Noto,Piboto,Arial,Helvetica,Sans")
        .Color(LvtkColor(0, 0, 0, 0.6))
        .FontSize(LvtkMeasurement::Point(12))
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);

    auto inner = std::make_shared<LvtkVerticalStackElement>();
    inner->Style()
        .Background(LvtkPattern(LvtkColor(0, 0.5, 0.5)))
        .BorderColor(LvtkColor(0, 0, 0.5, 0.6))
        .BorderWidth(4)
        .Margin(4)
        .Padding(4)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);
    result->AddChild(inner);

    auto inner2 = std::make_shared<LvtkVerticalStackElement>();
    inner2->Style()
        .Background(LvtkPattern(LvtkColor(0.5, 0.0, 0.5)))
        .BorderColor(LvtkColor(0, 0, 0.0, 0.6))
        .BorderWidth(4)
        .Margin(4)
        .Padding(4)
        .HorizontalAlignment(LvtkAlignment::End);
    inner->AddChild(inner2);

    auto text = std::make_shared<LvtkTypographyElement>();

    text->Text("Abc <b>def</b> ghi");
    text->Style()
        .Background(LvtkColor(1, 1, 1, 0.5))
        .Margin(4)
        .BorderWidth(4)
        .Padding(4)
        .BorderColor(LvtkColor(0, 0, 0, 0.6));

    text->MouseDown.AddListener(
        [text]( const LvtkMouseEventArgs &event) -> bool
        {
            text->Style().Color(LvtkColor(1, 0, 0, 1.0));
            text->Invalidate();
            text->CaptureMouse();
            return true;
        });
    text->MouseUp.AddListener(
        [text]( const LvtkMouseEventArgs &event) -> bool
        {
            text->Style().Color(LvtkColor(0, 0, 0, 0.6));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        });

    text->MouseOver.AddListener(
        [text]( const LvtkMouseOverEventArgs &event) -> bool
        {
            text->Style().Background(LvtkColor(1, 1, 1, 1));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        }

    );
    text->MouseOut.AddListener(
        [text]( const LvtkMouseOverEventArgs &event) -> bool
        {
            text->Style().Background(LvtkColor(1, 1, 1, 0.6));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        });

    inner2->AddChild(text);

    auto  percent = LvtkVerticalStackElement::Create();
    inner->AddChild(percent);
    percent->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(LvtkColor(0,0,0,0.4))
        .Background(LvtkColor(0.5,0.5,0.0,1));

    auto percent2 = LvtkVerticalStackElement::Create();
    percent->AddChild(percent2);
    percent2->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .VerticalAlignment(LvtkAlignment::Start)
        .Width(LvtkMeasurement::Percent(100))
        .Height(LvtkMeasurement::Percent(100))
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(LvtkColor(0,0,0,0.4))
        .Background(LvtkColor(0.1,0.5,0.0,1));

    auto percent3 = LvtkVerticalStackElement::Create();
    percent2->AddChild(percent3);
    percent3->Style()
        .HorizontalAlignment(LvtkAlignment::End)
        .VerticalAlignment(LvtkAlignment::Center)
        .Width(LvtkMeasurement::Percent(50))
        .Height(LvtkMeasurement::Percent(50))
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(LvtkColor(0,0,0,1.0))
        .Background(LvtkColor(0.1,0.0,0.75,1));


    return std::static_pointer_cast<LvtkElement>(result);
}
