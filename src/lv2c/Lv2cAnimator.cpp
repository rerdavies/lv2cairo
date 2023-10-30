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

#include "lv2c/Lv2cAnimator.hpp"
#include "lv2c/Lv2cWindow.hpp"

using namespace lv2c;

Lv2cAnimator::~Lv2cAnimator()
{
}

void Lv2cAnimator::Initialize(
    Lv2cElement *owner,
    double timeInSeconds,
    double timeoutSeconds,
    std::function<void(double value)> &&callback,
    double initialValue)
{
    using namespace std::chrono;
    Initialize(
        owner,
        duration_cast<clock_t::duration>(std::chrono::duration<double>(timeInSeconds)),
        duration_cast<clock_t::duration>(std::chrono::duration<double>(timeInSeconds)),
        std::move(callback),
        initialValue);
}

void Lv2cAnimator::Initialize(
    Lv2cElement *owner,
    clock_t::duration timeIn,
    clock_t::duration timeOut,
    std::function<void(double value)> &&callback,
    double initialValue)
{
    if (this->owner != nullptr)
    {
        throw std::runtime_error("Already initialized.");
    }
    this->callback = std::move(callback);
    this->owner = owner;
    this->currentValue = this->targetValue = initialValue;
    tickRateIn = timeIn.count() > 0 ? 1.0 / timeIn.count() : 0;
    tickRateOut = timeOut.count() > 0 ? 1.0 / timeOut.count() : 0;

    owner->OnMounted.AddListener([this](Lv2cWindow *window)
                                 { OnOwnerMounted(window); return false; });

    owner->OnUnmounted.AddListener([this](Lv2cWindow *window)
                                   { OnOwnerUnmounted(window); return false; });
    if (owner->IsMounted())
    {
        OnOwnerMounted(owner->Window());
    }
}

void Lv2cAnimator::SetTarget(double value)
{
    targetValue = value;
    if (this->currentValue != targetValue)
    {
        if (ownerMounted)
        {
            StartAnimation();
        }
    }
}

void Lv2cAnimator::OnOwnerMounted(Lv2cWindow *window)
{
    ownerMounted = true;
    if (targetValue != currentValue && !animationHandle)
    {
        StartAnimation();
    }
}
void Lv2cAnimator::StartAnimation()
{
    if (!animationHandle)
    {
        if (ownerMounted)
        {
            this->lastAnimationTime = clock_t::now();
            animationHandle = owner->Window()->RequestAnimationCallback(
                [this](clock_t::time_point now)
                { AnimationTick(now); });
        }
    }
}

void Lv2cAnimator::StopAnimation()
{
    if (animationHandle)
    {
        this->owner->Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
        currentValue = targetValue;
        callback(currentValue);
    }
}
void Lv2cAnimator::OnOwnerUnmounted(Lv2cWindow *window)
{
    ownerMounted = false;
    StopAnimation();
}
void Lv2cAnimator::AnimationTick(clock_t::time_point now)
{
    clock_t::duration::rep dTicks = (now - lastAnimationTime).count();
    lastAnimationTime = now;
    this->animationHandle = AnimationHandle::InvalidHandle;

    if (currentValue < targetValue)
    {
        if (tickRateIn <= 0)
        {
            currentValue = targetValue;
            callback(easingFunction(currentValue));
            return;
        }
        double dV = dTicks * tickRateIn;
        double newValue = currentValue + dV;
        if (newValue >= targetValue)
        {
            currentValue = targetValue;
            callback(easingFunction(currentValue));
            return;
        }
        currentValue = newValue;
        callback(easingFunction(currentValue));
    }
    else
    {
        if (tickRateOut <= 0)
        {
            currentValue = targetValue;
            callback(easingFunction(currentValue));
            return;
        }
        double dV = dTicks * tickRateOut;
        double newValue = currentValue - dV;
        if (newValue <= targetValue)
        {
            currentValue = targetValue;
            callback(easingFunction(currentValue));
            return;
        }
        currentValue = newValue;
        callback(easingFunction(currentValue));
    }
    this->animationHandle = owner->Window()->RequestAnimationCallback(
        [this](clock_t::time_point now)
        { AnimationTick(now); });
}

void Lv2cAnimator::Animate(double from, double to)
{
    currentValue = from;
    targetValue = to;
    StartAnimation();
}

void Lv2cAnimator::CancelAnimation()
{
    StopAnimation();
}

void Lv2cAnimator::Animate(double from, double to, double seconds)
{
    using namespace std::chrono;
    Animate(from, to, duration_cast<clock_t::duration>(duration<double>(seconds)));
}

void Lv2cAnimator::Animate(double from, double to, clock_t::duration time)
{
    using namespace std::chrono;

    auto ticks = time.count();
    double rate;
    if (ticks <= 0)
    {
        rate = 0;
    }
    else
    {
        rate = (to - from) / ticks;
    }
    tickRateIn = tickRateOut = rate;

    currentValue = from;
    targetValue = to;

    StartAnimation();
}

void Lv2cAnimator::EasingFunction(Lv2cEasingFunction easingFunction)
{
    switch (easingFunction)
    {
    case Lv2cEasingFunction::Linear:
        EasingFunction([](double v)
                          { return v; });
    break;
    case Lv2cEasingFunction::EaseIn:
        EasingFunction(
            [](double v)
            {
                if (v > 1)
                    return 1.0;
                if (v < 0)
                    return 0.0;
                double t = 1 - v;
                double result = std::sqrt(1 - t * t);
                return result;
            });
        break;
    case Lv2cEasingFunction::EaseInQuint:
        EasingFunction(
            [](double v)
            {
                if (v > 1)
                    return 1.0;
                if (v < 0)
                    return 0.0;
                return 1-std::pow(1-v,5);
            });
        break;
    case Lv2cEasingFunction::EaseInQuad:
        EasingFunction(
            [](double v)
            {
                if (v > 1)
                    return 1.0;
                if (v < 0)
                    return 0.0;
                return 1-std::pow(1-v,4);
            });
        break;
    case Lv2cEasingFunction::EaseOut:
        EasingFunction(
            [](double v)
            {
                if (v > 1)
                    return 1.0;
                if (v < 0)
                    return 0.0;
                double t = v;
                double result = std::sqrt(1 - t * t);
                return result;
            });
        break;
    }
}
void Lv2cAnimator::EasingFunction(easing_function_t &&function)
{
    this->easingFunction = std::move(function);
}

static inline double lerp(double t, double x0, double x1)
{
    return x0  + t*(x1-x0);
}

static double BezierY(double x, Lv2cPoint p0, Lv2cPoint p1, Lv2cPoint p2, Lv2cPoint p3)
{
    // NB: doesn't deal with bezier curves that have more than one value at any give X,
    // which makes it a strange choice. But what the heck, css uses it for easing functions, so we do to.
    double xMin = 0; 
    double xMax = 1;
    // avoid rounding errors 
    if (x == 0) return 0;
    if (x == 1) return 1;

    double xMid;
    while (true)
    {
        xMid = (xMax+xMin)*0.5;
        if (xMax-xMin < 1E-7) break;
        
        double bezierX = lerp(xMid,
            lerp(xMid,p0.x,p1.x),
            lerp(xMid,p2.x,p3.x));
        if (bezierX > x)
        {
            xMax = xMid;
        } else {
            xMin = xMid;
        }
    }
    return lerp(
        xMid,
        lerp(xMid,p0.y,p1.y),
        lerp(xMid,p2.y,p3.y));
}


void Lv2cAnimator::BezierEasingFunction(Lv2cPoint p1, Lv2cPoint p2)
{
    EasingFunction([p1,p2] (double x) {
        return BezierY(x, Lv2cPoint(0,0),p1,p2,Lv2cPoint(1,1));
    });
}



