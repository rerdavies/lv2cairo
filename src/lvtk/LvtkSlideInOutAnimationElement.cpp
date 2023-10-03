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

#include "lvtk/LvtkSlideInOutAnimationElement.hpp"

using namespace lvtk;

LvtkSlideInOutAnimationElement::LvtkSlideInOutAnimationElement()
{
    animator.Initialize(this, 1, 1,
                        [this](double value)
                        {
                            AnimationTick(value);
                        });
}

static LvtkPoint Blend(double value, LvtkPoint p0, LvtkPoint p1)
{
    return LvtkPoint(
        p0.x * (1 - value) + value * p1.x,
        p0.y * (1 - value) + value * p1.y);
}
void LvtkSlideInOutAnimationElement::AnimationTick(double value)
{
    // std::cout << "Animation value: " << value << std::endl;

    LvtkPoint newPoint = Blend(value, startPoint, endPoint);

    LvtkPoint lastPoint = this->lastPoint;
    constexpr double MAX_BLUR = 150;
    if (newPoint.y - lastPoint.y > MAX_BLUR)
    {
        lastPoint.y = newPoint.y - MAX_BLUR;
    }
    else if (newPoint.y - lastPoint.y < -MAX_BLUR)
    {
        lastPoint.y = newPoint.y + MAX_BLUR;
    }
    if (newPoint.x - lastPoint.x > MAX_BLUR)
    {
        lastPoint.x = newPoint.x - MAX_BLUR;
    }
    else if (newPoint.x - lastPoint.x < -MAX_BLUR)
    {
        lastPoint.x = newPoint.x + MAX_BLUR;
    }

    if (value >= 1.0)
    {
        newPoint = endPoint;
        // last frame, with blur.
        super::Blur(lastPoint, newPoint);
        if (lastPoint != endPoint)
        {
            animator.Animate(1.1, 1.1000001); // Ask for one more.
        }
    }
    else
    {
        super::Blur(lastPoint, newPoint);
    }
    this->lastPoint = newPoint;
}


void LvtkSlideInOutAnimationElement::StartAnimation(LvtkSlideAnimationType type, double milliseconds)
{
    using namespace std::chrono;

    this->animationType = type;
    this->animationLengthMs = milliseconds;
    double width = ClientSize().Width();
    double height = ClientSize().Height();
    if (this->LayoutValid())
    {
        switch (type)
        {
        case LvtkSlideAnimationType::None:
            startPoint = LvtkPoint(0, 0);
            endPoint = LvtkPoint(0, 0);
            break;
        case LvtkSlideAnimationType::SlideInStart:
            startPoint = LvtkPoint(-width, 0);
            endPoint = LvtkPoint(0, 0);
            break;
        case LvtkSlideAnimationType::SlideInEnd:
            startPoint = LvtkPoint(width, 0);
            endPoint = LvtkPoint(0, 0);
            break;
        case LvtkSlideAnimationType::SlideInTop:
            startPoint = LvtkPoint(0, -height);
            endPoint = LvtkPoint(0, 0);
            break;
        case LvtkSlideAnimationType::SlideInBottom:
            startPoint = LvtkPoint(0, height);
            endPoint = LvtkPoint(0, 0);
            break;
        case LvtkSlideAnimationType::SlideOutStart:
            startPoint = LvtkPoint(0, 0);
            endPoint = LvtkPoint(-width, 0);
            break;
        case LvtkSlideAnimationType::SlideOutEnd:
            startPoint = LvtkPoint(0, 0);
            endPoint = LvtkPoint(width, 0);
            break;
        case LvtkSlideAnimationType::SlideOutBottom:
            startPoint = LvtkPoint(0, 0);
            endPoint = LvtkPoint(0, height);
            break;
        case LvtkSlideAnimationType::SlideOutTop:
            startPoint = LvtkPoint(0, 0);
            endPoint = LvtkPoint(0, -height);
            break;
        }
        lastPoint = startPoint;
        animator.Animate(0, 1, duration_cast<animation_clock_t::duration>(duration<double>(milliseconds * 0.001)));
        AnimationTick(0);
    }
    else
    {
        this->pendingRequest = true;
    }
}

void LvtkSlideInOutAnimationElement::OnLayoutComplete()
{
    if (this->pendingRequest)
    {
        this->pendingRequest = false;
        StartAnimation(animationType, animationLengthMs);
    }
}

void LvtkSlideInOutAnimationElement::EasingFunction(LvtkEasingFunction easingFunction) { animator.EasingFunction(easingFunction); }
void LvtkSlideInOutAnimationElement::EasingFunction(std::function<double(double)> &&easingFunction) { animator.EasingFunction(std::move(easingFunction)); }
void LvtkSlideInOutAnimationElement::BezierEasingFunction(LvtkPoint p1, LvtkPoint p2) { animator.BezierEasingFunction(p1, p2); }
