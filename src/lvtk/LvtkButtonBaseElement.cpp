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

#include "lvtk/LvtkButtonBaseElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include <chrono>

#include "lvtk/LvtkDrawingContext.hpp"

using namespace lvtk;

LvtkButtonBaseElement::LvtkButtonBaseElement()
{
    this->EnabledProperty.SetElement(this, &LvtkButtonBaseElement::OnEnabledChanged);
}

void LvtkButtonBaseElement::OnHoverStateChanged(LvtkHoverState state)
{
    Invalidate();
}

inline double DSquared(LvtkPoint p0, LvtkPoint p1)
{
    double dx = p0.x - p1.x;
    double dy = p0.y - p1.y;
    return dx * dx + dy * dy;
}

const LvtkHoverColors &LvtkButtonBaseElement::HoverBackgroundColors()
{
    return this->Theme().hoverBackgroundColors;
}
void LvtkButtonBaseElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    if (Enabled())
    {
        auto hoverState = HoverState();
        if (!ShowPressedState())
        {
            hoverState = hoverState - LvtkHoverState::Pressed;
        }
        auto hoverColor = HoverBackgroundColors().GetColor(hoverState);

        this->drawRoundInsideBorderRect(dc);
        dc.set_source(hoverColor);
        dc.fill();
    }
    if (animationValue != 0)
    {
        LvtkRectangle rcb = this->getBorderBounds();
        // maximum distance from the click point to the four corners of the element.
        double radius =
            std::sqrt(
                std::max(
                    std::max(
                        DSquared(LvtkPoint(rcb.Left(), rcb.Top()), animationCenter),
                        DSquared(LvtkPoint(rcb.Left(), rcb.Bottom()), animationCenter)),
                    std::max(
                        DSquared(LvtkPoint(rcb.Right(), rcb.Top()), animationCenter),
                        DSquared(LvtkPoint(rcb.Right(), rcb.Bottom()), animationCenter))));

        radius *= 1.2;
        LvtkColor c0 = HoverBackgroundColors().GetColor(LvtkHoverState::Pressed);
        LvtkColor c1 = HoverBackgroundColors().GetColor(LvtkHoverState::Empty);

        LvtkPattern radialGradient = LvtkPattern::radial_gradient(
            animationCenter.x, animationCenter.y,
            radius,
            {LvtkColorStop{0, c0},
             LvtkColorStop(animationValue, c0),
             LvtkColorStop(animationValue + 0.3, c1)});
        this->drawRoundInsideBorderRect(dc);
        dc.set_source(radialGradient);
        dc.fill();
    }
}

void LvtkButtonBaseElement::CancelAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
}

bool LvtkButtonBaseElement::OnMouseMove(LvtkMouseEventArgs &event)
{
    if (super::OnMouseMove(event))
        return true;
    if (this->Hascapture())
    {
        if (this->ScreenBounds().Contains(event.screenPoint))
        {
            this->HoverState(HoverState() + LvtkHoverState::Pressed);
        }
        else
        {
            this->HoverState(HoverState() - LvtkHoverState::Pressed);
        }
        return true;
    }
    return false;
}

bool LvtkButtonBaseElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    if (!Enabled())
        return false;
    this->animationCenter = event.point;
    StartAnimation(true);
    this->CaptureMouse();
    this->HoverState(HoverState() + LvtkHoverState::Pressed);
    return true;
}

bool LvtkButtonBaseElement::OnClick(LvtkMouseEventArgs &event)
{
    return this->Clicked.Fire(event);
}

bool LvtkButtonBaseElement::OnMouseUp(LvtkMouseEventArgs &event)
{
    if (Hascapture())
    {
        this->HoverState(HoverState() - LvtkHoverState::Pressed);
        this->ReleaseCapture();
        StartAnimation(false);

        if (ScreenBorderRect().Contains(event.screenPoint))
        {
            if (this->clickEventHandle)
            {
                Window()->CancelPostDelayed(this->clickEventHandle);
                this->clickEventHandle = AnimationHandle::InvalidHandle;
            }
            LvtkMouseEventArgs tEvent = event;
            // call OnClick AFTER X11 has released the mouse capture. (unpleasant debugging experience)
            this->clickEventHandle = Window()->PostDelayed(
                0,
                [this, tEvent]() mutable
                { 
                    this->CheckValid();
                    OnClick(tEvent); 
                    this->clickEventHandle = AnimationHandle::InvalidHandle;
                });
        }
        return true;
    }
    return false;
}

void LvtkButtonBaseElement::AnimationValue(double value)
{
    if (this->animationValue != value)
    {
        this->animationValue = value;
        Invalidate();
    }
}

void LvtkButtonBaseElement::AnimationTick(const animation_clock_time_point_t&now)
{
    std::chrono::duration<double> fs = now - animationStartTime;

    this->animationHandle = AnimationHandle::InvalidHandle;
    double t = fs.count();
    if (animationIncreasing)
    {
        constexpr double increase_seconds = 0.250;
        constexpr double increase_rate = 1 / increase_seconds;
        double value = this->animationStartValue + increase_rate * t;
        if (value >= 1.0)
        {
            value = 1.0;
            AnimationValue(value);
            return;
        }
        AnimationValue(value);
        RequestAnimationTick();
    }
    else
    {
        constexpr double increase_seconds = 0.0700;
        constexpr double increase_rate = 1.0 / increase_seconds;
        double value = increase_seconds * t + this->animationStartValue - increase_rate * t;
        if (value <= 0)
        {
            value = 0;
            AnimationValue(0);
            return;
        }
        AnimationValue(value);
        RequestAnimationTick();
    }
}

void LvtkButtonBaseElement::RequestAnimationTick()
{
    if (IsMounted())
    {
        this->animationHandle = Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                this->AnimationTick(now);
            });
    }
}
void LvtkButtonBaseElement::StartAnimation(bool increasing)
{
    CancelAnimation();
    this->animationStartValue = this->animationValue;
    this->animationStartTime = clock_t::now();
    this->animationIncreasing = increasing;
    RequestAnimationTick();
}

bool LvtkButtonBaseElement::OnFocus(const LvtkFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    HoverState(HoverState() + LvtkHoverState::Focus);
    return false;
}
bool LvtkButtonBaseElement::OnLostFocus(const LvtkFocusEventArgs &eventArgs)
{
    HoverState(HoverState() - LvtkHoverState::Focus);
    super::OnLostFocus(eventArgs);

    return false;
}

void LvtkButtonBaseElement::OnMount()
{
    animationValue = 0;
    if (Enabled())
    {
        RemoveClass(Theme().buttonDisableStyle);
    }
    else
    {
        AddClass(Theme().buttonDisableStyle);
    }
}
void LvtkButtonBaseElement::OnUnmount()
{
    RemoveClass(Theme().buttonDisableStyle);
    if (clickEventHandle)
    {
        Window()->CancelPostDelayed(clickEventHandle);
        clickEventHandle = AnimationHandle::InvalidHandle;
    }
    CancelAnimation();
    animationValue = 0;
}
void LvtkButtonBaseElement::OnEnabledChanged(bool enable)
{
    if (Window())
    {
        if (Enabled())
        {
            RemoveClass(Theme().buttonDisableStyle);
        }
        else
        {
            AddClass(Theme().buttonDisableStyle);
        }
        Invalidate();
    }
}

bool LvtkButtonBaseElement::ShowPressedState() const
{
    return false;
}
