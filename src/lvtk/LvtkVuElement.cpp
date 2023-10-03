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

#include "lvtk/LvtkVuElement.hpp"

using namespace lvtk;

void LvtkVuElement::OnMount()
{
    UpdateStyle();
}
void LvtkStereoVuElement::OnMount()
{
    UpdateStyle();
}
void LvtkVuElement::UpdateStyle()
{
    Classes(Theme().vuStyle);
    InvalidateLayout();
}
void LvtkStereoVuElement::UpdateStyle()
{
    Classes(Theme().stereoVuStyle);
    InvalidateLayout();
}
const LvtkVuSettings &LvtkVuElement::Settings() const
{
    return Theme().vuSettings;
}
const LvtkVuSettings &LvtkStereoVuElement::Settings() const
{
    return Theme().vuSettings;
}

/*static*/
double LvtkVuElement::ValueToClient(double value, double minValue, double maxValue, const LvtkRectangle &vuRectangle)
{
    double v = (value - minValue) / (maxValue-minValue);
    v = vuRectangle.Top() + (1 - v) * vuRectangle.Height();
    if (v > vuRectangle.Bottom())
        v = vuRectangle.Bottom();
    if (v < vuRectangle.Top())
        v = vuRectangle.Top();
    return v;
}

void LvtkVuElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    const LvtkVuSettings &settings = Settings();

    LvtkRectangle clientRectangle{this->ClientSize()};
    LvtkRectangle deviceRect = dc.user_to_device(clientRectangle).Ceiling();
    clientRectangle = dc.device_to_user(deviceRect);

    LvtkRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    if (settings.hasTicks)
    {
        double offsetX = settings.tickWidth + settings.padding;

        vuRectangle = LvtkRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());
    }
    DrawVu(dc,Value(),MinValue(),MaxValue(), vuRectangle, Settings());
}
void LvtkStereoVuElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    const LvtkVuSettings &settings = Settings();

    LvtkRectangle clientRectangle{this->ClientSize()};
    LvtkRectangle deviceRect = dc.user_to_device(clientRectangle).Ceiling();
    clientRectangle = dc.device_to_user(deviceRect);

    LvtkRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    if (settings.hasTicks)
    {
        double offsetX = settings.tickWidth + settings.padding;

        vuRectangle = LvtkRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());
    }
    double vuWidth = (vuRectangle.Width()-settings.padding)/2;

    LvtkRectangle leftVu { vuRectangle.Left(),vuRectangle.Top(),vuWidth,vuRectangle.Height()};
    LvtkVuElement::DrawVu(dc,Value(),MinValue(),MaxValue(), leftVu, Settings());

    LvtkRectangle rightVu { vuRectangle.Right()-vuWidth,vuRectangle.Top(),vuWidth,vuRectangle.Height()};
    LvtkVuElement::DrawVu(dc,RightValue(),MinValue(),MaxValue(), rightVu, Settings());

}


void LvtkVuElement::DrawVu(
    LvtkDrawingContext &dc,
    double value,
    double minValue,
    double maxValue,
    const LvtkRectangle &vuRectangle,
    const LvtkVuSettings &settings)
{

    double level = ValueToClient(value, minValue,maxValue, vuRectangle);
    double redLevel = -1;
    if (settings.redLevel.has_value())
    {
        redLevel = ValueToClient(settings.redLevel.value(),minValue,maxValue, vuRectangle);
    }
    double yellowLevel = -1;
    if (settings.yellowLevel.has_value())
    {
        yellowLevel = ValueToClient(settings.yellowLevel.value(), minValue,maxValue,vuRectangle);
    }
    if (redLevel != -1 && yellowLevel == -1)
    {
        yellowLevel = redLevel;
    }
    if (level >= vuRectangle.Bottom() - 1) // always display something.
    {
        level = vuRectangle.Bottom() - 1;
    }

    if (level >= yellowLevel)
    {
        dc.set_source(settings.green);
        if (!settings.yellowLevel.has_value() && !settings.redLevel.has_value() // is it a non-db VU?
            && minValue < 0 && maxValue > 0) // is it bivalent?
        {
            auto zeroLevel = ValueToClient(0,minValue,maxValue, vuRectangle);
            if (std::abs(level-zeroLevel) < 1)
            {
                dc.rectangle(LvtkRectangle(
                    vuRectangle.Left(), 
                    std::min(level,zeroLevel), vuRectangle.Width(), 1)
                );
            }
            if (level < zeroLevel)
            {
                dc.rectangle(LvtkRectangle(vuRectangle.Left(), level, vuRectangle.Width(), zeroLevel-level));
            } else {
                dc.rectangle(LvtkRectangle(vuRectangle.Left(), zeroLevel, vuRectangle.Width(), level - zeroLevel));
            }
            

        } else {
            dc.rectangle(LvtkRectangle(vuRectangle.Left(), level, vuRectangle.Width(), vuRectangle.Bottom() - level));
        }
        dc.fill();
    }
    else
    {
        dc.set_source(settings.green);
        dc.rectangle(LvtkRectangle(vuRectangle.Left(), yellowLevel, vuRectangle.Width(), vuRectangle.Bottom() - yellowLevel));
        dc.fill();

        if (level >= redLevel)
        {
            dc.set_source(settings.yellow);
            dc.rectangle(LvtkRectangle(vuRectangle.Left(), level, vuRectangle.Width(), yellowLevel - level));
            dc.fill();
        }
        else
        {
            dc.set_source(settings.yellow);
            dc.rectangle(LvtkRectangle(vuRectangle.Left(), redLevel, vuRectangle.Width(), yellowLevel - redLevel));
            dc.fill();

            dc.set_source(settings.red);
            dc.rectangle(LvtkRectangle(vuRectangle.Left(), level, vuRectangle.Width(), redLevel - level));
            dc.fill();
        }
    }
}

void LvtkVuElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
    Invalidate();
}

void LvtkStereoVuElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
    Invalidate();
}

void LvtkStereoVuElement::OnRightValueChanged(double value)
{
    super::OnValueChanged(value);
    Invalidate();
}