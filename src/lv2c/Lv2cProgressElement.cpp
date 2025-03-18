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

#include "lv2c/Lv2cProgressElement.hpp"

using namespace lv2c;

void Lv2cProgressElement::OnMount()
{
    UpdateStyle();
}
void Lv2cProgressElement::UpdateStyle()
{
    Classes(Theme().progressStyle);
    InvalidateLayout();
}
const Lv2cVuSettings &Lv2cProgressElement::Settings() const
{
    return Theme().vuSettings;
}

/*static*/
double Lv2cProgressElement::ValueToClient(double value, double minValue, double maxValue, const Lv2cRectangle &vuRectangle)
{
    double v = (value - minValue) / (maxValue-minValue);
    if (v < 0) v = 0; 
    if (v > 1) v = 1;
    return v * vuRectangle.Width();
}

void Lv2cProgressElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    const Lv2cVuSettings &settings = Settings();

    Lv2cRectangle clientRectangle{this->ClientSize()};
    Lv2cRectangle deviceRect = dc.user_to_device(clientRectangle).Ceiling();
    clientRectangle = dc.device_to_user(deviceRect);

    Lv2cRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    DrawVu(dc,Value(),MinValue(),MaxValue(), vuRectangle, Settings());
}

void Lv2cProgressElement::DrawVu(
    Lv2cDrawingContext &dc,
    double value,
    double minValue,
    double maxValue,
    const Lv2cRectangle &vuRectangle,
    const Lv2cVuSettings &settings)
{

    double level = ValueToClient(value, minValue,maxValue, vuRectangle);
    
    dc.rectangle(Lv2cRectangle(vuRectangle.Left(), vuRectangle.Top(), level, vuRectangle.Height()));
    dc.set_source(settings.green);
    dc.fill();
}

void Lv2cProgressElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
    Invalidate();
}

