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

#include "lvtk/LvtkDialBaseElement.hpp"
#include "lvtk/LvtkWindow.hpp"

using namespace lvtk;

LvtkDialBaseElement::LvtkDialBaseElement()
{
    this->HoverOpacityProperty.SetElement(this,&LvtkDialBaseElement::OnHoverOpacityChanged);
}

void LvtkDialBaseElement::OnHoverOpacityChanged(const std::optional<LvtkHoverOpacity>&value)
{
    OnHoverStateChanged(this->HoverState());
}

void LvtkDialBaseElement::OnMount()
{
    OnHoverStateChanged(this->HoverState());
}

void LvtkDialBaseElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
}

bool LvtkDialBaseElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    HoverState(HoverState() + LvtkHoverState::Pressed);

    lastMousePoint = event.point;
    mouseValue = Value();
    this->CaptureMouse();
    IsDraggingProperty.set(true);
    return true;
}
bool LvtkDialBaseElement::OnMouseUp(LvtkMouseEventArgs &event)
{
    IsDraggingProperty.set(false);
    if (Hascapture())
    {
        this->ReleaseCapture();

        HoverState(HoverState() - LvtkHoverState::Pressed);
        UpdateMousePoint(event);
    }
    return true;
}
bool LvtkDialBaseElement::OnMouseMove(LvtkMouseEventArgs &event)
{
    if (Hascapture())
    {

        UpdateMousePoint(event);
    }
    return true;
}

void LvtkDialBaseElement::UpdateMousePoint(LvtkMouseEventArgs &event)
{
    LvtkPoint point = event.point;

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

const LvtkHoverOpacity&LvtkDialBaseElement::GetHoverOpacity() const
{
    if (HoverOpacity().has_value())
    {
        return HoverOpacity().value();
    }
    return Theme().dialHoverOpacity;
}

void LvtkDialBaseElement::OnHoverStateChanged(LvtkHoverState state)
{
    if (!IsMounted()) return;

    super::OnHoverStateChanged(state);

    double opacity = GetHoverOpacity().GetOpacity(state);
    OnDialOpacityChanged(opacity);

}

void LvtkDialBaseElement::OnDialOpacityChanged(double opacity)
{

}
