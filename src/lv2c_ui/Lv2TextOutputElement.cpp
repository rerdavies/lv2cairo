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

#include "lv2c_ui/Lv2TextOutputElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cPangoContext.hpp"
#include "lv2c/Lv2cDropdownItemElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cPangoContext.hpp"

#include <chrono>


using namespace lv2c;
using namespace lv2c::ui;


Lv2cSize Lv2TextOutputElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context)
{
    return super::MeasureClient(clientConstraint,clientAvailable,context);
}
Lv2TextOutputElement::~Lv2TextOutputElement() noexcept
{
}

Lv2TextOutputElement::Lv2TextOutputElement(const Lv2PortInfo*portInfo)
: portInfo(portInfo)
{
    typography = Lv2cTypographyElement::Create();
    typography->Variant(Lv2cTypographyVariant::BodySecondary);
    this->AddChild(typography);

    typography->Style().HorizontalAlignment(Lv2cAlignment::Stretch)
        .SingleLine(true)
        .Ellipsize(Lv2cEllipsizeMode::End)
        .Width(120);
        ;

    DisplayValueProperty.Bind(typography->TextProperty);

}

void Lv2TextOutputElement::OnMount()
{
    auto &theme = Theme();
    this->AddClass(theme.statusTextStyle);
}



