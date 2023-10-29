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

#include "VerticalStackTest.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"

using namespace lvtk;

Lv2cElement::ptr VerticalStackTestPage::CreatePageView(Lv2cTheme::ptr theme)
{
    auto result = std::make_shared<Lv2cVerticalStackElement>();
    result->Style()
        .Background(Lv2cPattern(Lv2cColor(1, 0.5, 0.5)))
        .BorderColor(Lv2cColor(0, 0, 0, 0.6))
        .BorderWidth(4.0)
        .Margin(4)
        .Padding(4)
        .FontFamily("Roboto,Noto,Piboto,Arial,Helvetica,Sans")
        .Color(Lv2cColor(0, 0, 0, 0.6))
        .FontSize(Lv2cMeasurement::Point(12))
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch);

    auto inner = std::make_shared<Lv2cVerticalStackElement>();
    inner->Style()
        .Background(Lv2cPattern(Lv2cColor(0, 0.5, 0.5)))
        .BorderColor(Lv2cColor(0, 0, 0.5, 0.6))
        .BorderWidth(4)
        .Margin(4)
        .Padding(4)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch);
    result->AddChild(inner);

    auto inner2 = std::make_shared<Lv2cVerticalStackElement>();
    inner2->Style()
        .Background(Lv2cPattern(Lv2cColor(0.5, 0.0, 0.5)))
        .BorderColor(Lv2cColor(0, 0, 0.0, 0.6))
        .BorderWidth(4)
        .Margin(4)
        .Padding(4)
        .HorizontalAlignment(Lv2cAlignment::End);
    inner->AddChild(inner2);

    auto text = std::make_shared<Lv2cTypographyElement>();

    text->Text("Abc <b>def</b> ghi");
    text->Style()
        .Background(Lv2cColor(1, 1, 1, 0.5))
        .Margin(4)
        .BorderWidth(4)
        .Padding(4)
        .BorderColor(Lv2cColor(0, 0, 0, 0.6));

    text->MouseDown.AddListener(
        [text]( const Lv2cMouseEventArgs &event) -> bool
        {
            text->Style().Color(Lv2cColor(1, 0, 0, 1.0));
            text->Invalidate();
            text->CaptureMouse();
            return true;
        });
    text->MouseUp.AddListener(
        [text]( const Lv2cMouseEventArgs &event) -> bool
        {
            text->Style().Color(Lv2cColor(0, 0, 0, 0.6));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        });

    text->MouseOver.AddListener(
        [text]( const Lv2cMouseOverEventArgs &event) -> bool
        {
            text->Style().Background(Lv2cColor(1, 1, 1, 1));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        }

    );
    text->MouseOut.AddListener(
        [text]( const Lv2cMouseOverEventArgs &event) -> bool
        {
            text->Style().Background(Lv2cColor(1, 1, 1, 0.6));
            text->Invalidate();
            text->ReleaseCapture();
            return true;
        });

    inner2->AddChild(text);

    auto  percent = Lv2cVerticalStackElement::Create();
    inner->AddChild(percent);
    percent->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(Lv2cColor(0,0,0,0.4))
        .Background(Lv2cColor(0.5,0.5,0.0,1));

    auto percent2 = Lv2cVerticalStackElement::Create();
    percent->AddChild(percent2);
    percent2->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .VerticalAlignment(Lv2cAlignment::Start)
        .Width(Lv2cMeasurement::Percent(100))
        .Height(Lv2cMeasurement::Percent(100))
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(Lv2cColor(0,0,0,0.4))
        .Background(Lv2cColor(0.1,0.5,0.0,1));

    auto percent3 = Lv2cVerticalStackElement::Create();
    percent2->AddChild(percent3);
    percent3->Style()
        .HorizontalAlignment(Lv2cAlignment::End)
        .VerticalAlignment(Lv2cAlignment::Center)
        .Width(Lv2cMeasurement::Percent(50))
        .Height(Lv2cMeasurement::Percent(50))
        .Margin({4})
        .Padding({4})
        .BorderWidth(4)
        .BorderColor(Lv2cColor(0,0,0,1.0))
        .Background(Lv2cColor(0.1,0.0,0.75,1));


    return std::static_pointer_cast<Lv2cElement>(result);
}
