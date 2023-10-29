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

#include "lv2c/Lv2cDialBaseElement.hpp"
#include "lv2c/Lv2cWindow.hpp"

using namespace lvtk;

Lv2cDialBaseElement::Lv2cDialBaseElement()
{
    this->HoverOpacityProperty.SetElement(this,&Lv2cDialBaseElement::OnHoverOpacityChanged);
}

void Lv2cDialBaseElement::OnHoverOpacityChanged(const std::optional<Lv2cHoverOpacity>&value)
{
    OnHoverStateChanged(this->HoverState());
}

void Lv2cDialBaseElement::OnMount()
{
    OnHoverStateChanged(this->HoverState());
}

void Lv2cDialBaseElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
}

bool Lv2cDialBaseElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    HoverState(HoverState() + Lv2cHoverState::Pressed);

    lastMousePoint = event.point;
    mouseValue = Value();
    this->CaptureMouse();
    IsDraggingProperty.set(true);
    return true;
}
bool Lv2cDialBaseElement::OnMouseUp(Lv2cMouseEventArgs &event)
{
    IsDraggingProperty.set(false);
    if (Hascapture())
    {
        this->ReleaseCapture();

        HoverState(HoverState() - Lv2cHoverState::Pressed);
        UpdateMousePoint(event);
    }
    return true;
}
bool Lv2cDialBaseElement::OnMouseMove(Lv2cMouseEventArgs &event)
{
    if (Hascapture())
    {

        UpdateMousePoint(event);
    }
    return true;
}

void Lv2cDialBaseElement::UpdateMousePoint(Lv2cMouseEventArgs &event)
{
    Lv2cPoint point = event.point;

    double dx = - (point.y-lastMousePoint.y); // (point.x-lastMousePoint.x)- (point.y-lastMousePoint.y);

    double rate = 1.0/200.;
    if (event.modifierState && ModifierState::Control)
    {
        rate /= 5;
    }
    if (event.modifierState && ModifierState::Shift)
    {
        rate /= 5;
    }
    double dValue = dx*rate;
    double newValue = mouseValue+dValue;
    if (newValue < 0) newValue = 0;
    if (newValue > 1) newValue = 1;

    // std::cout 
    // << "  screenPoint: " << event.screenPoint.toString() 
    // << "    point: " << point.toString() 
    // << " last: " << lastMousePoint.toString() 
    // << " dx: " << dx << "  v: " << newValue << std::endl;
    mouseValue = newValue;
    Value(newValue);

    lastMousePoint = point;
}

const Lv2cHoverOpacity&Lv2cDialBaseElement::GetHoverOpacity() const
{
    if (HoverOpacity().has_value())
    {
        return HoverOpacity().value();
    }
    return Theme().dialHoverOpacity;
}

void Lv2cDialBaseElement::OnHoverStateChanged(Lv2cHoverState state)
{
    if (!IsMounted()) return;

    super::OnHoverStateChanged(state);

    double opacity = GetHoverOpacity().GetOpacity(state);
    OnDialOpacityChanged(opacity);

}

void Lv2cDialBaseElement::OnDialOpacityChanged(double opacity)
{

}
