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

#include "lv2c/Lv2cButtonBaseElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include <chrono>

#include "lv2c/Lv2cDrawingContext.hpp"

using namespace lv2c;

Lv2cButtonBaseElement::Lv2cButtonBaseElement()
{
    this->EnabledProperty.SetElement(this, &Lv2cButtonBaseElement::OnEnabledChanged);
}

void Lv2cButtonBaseElement::OnHoverStateChanged(Lv2cHoverState state)
{
    Invalidate();
}

inline double DSquared(Lv2cPoint p0, Lv2cPoint p1)
{
    double dx = p0.x - p1.x;
    double dy = p0.y - p1.y;
    return dx * dx + dy * dy;
}

const Lv2cHoverColors &Lv2cButtonBaseElement::HoverBackgroundColors()
{
    return this->Theme().hoverBackgroundColors;
}
void Lv2cButtonBaseElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    if (Enabled())
    {
        auto hoverState = HoverState();
        if (!ShowPressedState())
        {
            hoverState = hoverState - Lv2cHoverState::Pressed;
        }
        auto hoverColor = HoverBackgroundColors().GetColor(hoverState);

        this->drawRoundInsideBorderRect(dc);
        dc.set_source(hoverColor);
        dc.fill();
    }
    if (animationValue != 0)
    {
        Lv2cRectangle rcb = this->getBorderBounds();
        // maximum distance from the click point to the four corners of the element.
        double radius =
            std::sqrt(
                std::max(
                    std::max(
                        DSquared(Lv2cPoint(rcb.Left(), rcb.Top()), animationCenter),
                        DSquared(Lv2cPoint(rcb.Left(), rcb.Bottom()), animationCenter)),
                    std::max(
                        DSquared(Lv2cPoint(rcb.Right(), rcb.Top()), animationCenter),
                        DSquared(Lv2cPoint(rcb.Right(), rcb.Bottom()), animationCenter))));

        radius *= 1.2;
        Lv2cColor c0 = HoverBackgroundColors().GetColor(Lv2cHoverState::Pressed);
        Lv2cColor c1 = HoverBackgroundColors().GetColor(Lv2cHoverState::Empty);

        Lv2cPattern radialGradient = Lv2cPattern::radial_gradient(
            animationCenter.x, animationCenter.y,
            radius,
            {Lv2cColorStop{0, c0},
             Lv2cColorStop(animationValue, c0),
             Lv2cColorStop(animationValue + 0.3, c1)});
        this->drawRoundInsideBorderRect(dc);
        dc.set_source(radialGradient);
        dc.fill();
    }
}

void Lv2cButtonBaseElement::CancelAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
}

bool Lv2cButtonBaseElement::OnMouseMove(Lv2cMouseEventArgs &event)
{
    if (super::OnMouseMove(event))
        return true;
    if (this->Hascapture())
    {
        if (this->ScreenBounds().Contains(event.screenPoint))
        {
            this->HoverState(HoverState() + Lv2cHoverState::Pressed);
        }
        else
        {
            this->HoverState(HoverState() - Lv2cHoverState::Pressed);
        }
        return true;
    }
    return false;
}

void Lv2cButtonBaseElement::CancelKeyboardDelay()
{
    if (keyboardDelayHandle)
    {
        this->Window()->CancelPostDelayed(keyboardDelayHandle);
        keyboardDelayHandle = AnimationHandle::InvalidHandle;

    }
}
void Lv2cButtonBaseElement::FireKeyboardClick()
{
    CancelKeyboardDelay();
    Lv2cMouseEventArgs eventArgs = MakeKeyboardEventArgs();
    this->Clicked.Fire(eventArgs);

    if (IsMounted())
    {
        StartAnimation(true);

        keyboardDelayHandle = this->Window()->PostDelayed(
            300,
            [this](){
                this->keyboardDelayHandle = AnimationHandle::InvalidHandle;
                StartAnimation(false);
            }
        );
    }
}


bool Lv2cButtonBaseElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    CancelKeyboardDelay();

    if (!Enabled())
        return false;
    this->animationCenter = event.point;
    StartAnimation(true);
    this->CaptureMouse();
    this->HoverState(HoverState() + Lv2cHoverState::Pressed);
    return true;
}

bool Lv2cButtonBaseElement::OnClick(Lv2cMouseEventArgs &event)
{
    return this->Clicked.Fire(event);
}

bool Lv2cButtonBaseElement::OnMouseUp(Lv2cMouseEventArgs &event)
{
    if (Hascapture())
    {
        this->HoverState(HoverState() - Lv2cHoverState::Pressed);
        this->ReleaseCapture();
        StartAnimation(false);

        if (ScreenBorderRect().Contains(event.screenPoint))
        {
            if (this->clickEventHandle)
            {
                Window()->CancelPostDelayed(this->clickEventHandle);
                this->clickEventHandle = AnimationHandle::InvalidHandle;
            }
            Lv2cMouseEventArgs tEvent = event;
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

void Lv2cButtonBaseElement::AnimationValue(double value)
{
    if (this->animationValue != value)
    {
        this->animationValue = value;
        Invalidate();
    }
}

void Lv2cButtonBaseElement::AnimationTick(const animation_clock_time_point_t&now)
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

void Lv2cButtonBaseElement::RequestAnimationTick()
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
void Lv2cButtonBaseElement::StartAnimation(bool increasing)
{
    CancelAnimation();
    if (!IsMounted())
    {
        AnimationValue(0);
        return;
    }
    this->animationStartValue = this->animationValue;
    this->animationStartTime = clock_t::now();
    this->animationIncreasing = increasing;
    RequestAnimationTick();
}

bool Lv2cButtonBaseElement::OnFocus(const Lv2cFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    HoverState(HoverState() + Lv2cHoverState::Focus);
    return false;
}
bool Lv2cButtonBaseElement::OnLostFocus(const Lv2cFocusEventArgs &eventArgs)
{
    HoverState(HoverState() - Lv2cHoverState::Focus);
    super::OnLostFocus(eventArgs);
    StartAnimation(false);

    return false;
}

void Lv2cButtonBaseElement::OnMount()
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
void Lv2cButtonBaseElement::OnUnmount()
{
    CancelKeyboardDelay();
    RemoveClass(Theme().buttonDisableStyle);
    if (clickEventHandle)
    {
        Window()->CancelPostDelayed(clickEventHandle);
        clickEventHandle = AnimationHandle::InvalidHandle;
    }
    CancelAnimation();
    animationValue = 0;
}
void Lv2cButtonBaseElement::OnEnabledChanged(bool enable)
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

Lv2cMouseEventArgs Lv2cButtonBaseElement::MakeKeyboardEventArgs()
{
    Lv2cMouseEventArgs eventArgs;
    eventArgs.button = 0;
    eventArgs.h = this->Window()->Handle();
    eventArgs.modifierState = ModifierState::Empty;

    Lv2cSize size = ClientSize();
    eventArgs.point = Lv2cPoint(size.Width()/2,size.Height()/2);
    Lv2cRectangle screenBounds = ScreenBounds();
    eventArgs.screenPoint = Lv2cPoint(
        screenBounds.Left() + screenBounds.Width()/2,
        screenBounds.Top() + screenBounds.Height()/2
    );
    return eventArgs;

}



bool Lv2cButtonBaseElement::ShowPressedState() const
{
    return false;
}
