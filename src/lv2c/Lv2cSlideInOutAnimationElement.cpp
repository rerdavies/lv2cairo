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

#include "lv2c/Lv2cSlideInOutAnimationElement.hpp"

using namespace lvtk;

Lv2cSlideInOutAnimationElement::Lv2cSlideInOutAnimationElement()
{
    animator.Initialize(this, 1, 1,
                        [this](double value)
                        {
                            AnimationTick(value);
                        });
}

static Lv2cPoint Blend(double value, Lv2cPoint p0, Lv2cPoint p1)
{
    return Lv2cPoint(
        p0.x * (1 - value) + value * p1.x,
        p0.y * (1 - value) + value * p1.y);
}
void Lv2cSlideInOutAnimationElement::AnimationTick(double value)
{
    // std::cout << "Animation value: " << value << std::endl;

    Lv2cPoint newPoint = Blend(value, startPoint, endPoint);

    Lv2cPoint lastPoint = this->lastPoint;
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


void Lv2cSlideInOutAnimationElement::StartAnimation(Lv2cSlideAnimationType type, double milliseconds)
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
        case Lv2cSlideAnimationType::None:
            startPoint = Lv2cPoint(0, 0);
            endPoint = Lv2cPoint(0, 0);
            break;
        case Lv2cSlideAnimationType::SlideInStart:
            startPoint = Lv2cPoint(-width, 0);
            endPoint = Lv2cPoint(0, 0);
            break;
        case Lv2cSlideAnimationType::SlideInEnd:
            startPoint = Lv2cPoint(width, 0);
            endPoint = Lv2cPoint(0, 0);
            break;
        case Lv2cSlideAnimationType::SlideInTop:
            startPoint = Lv2cPoint(0, -height);
            endPoint = Lv2cPoint(0, 0);
            break;
        case Lv2cSlideAnimationType::SlideInBottom:
            startPoint = Lv2cPoint(0, height);
            endPoint = Lv2cPoint(0, 0);
            break;
        case Lv2cSlideAnimationType::SlideOutStart:
            startPoint = Lv2cPoint(0, 0);
            endPoint = Lv2cPoint(-width, 0);
            break;
        case Lv2cSlideAnimationType::SlideOutEnd:
            startPoint = Lv2cPoint(0, 0);
            endPoint = Lv2cPoint(width, 0);
            break;
        case Lv2cSlideAnimationType::SlideOutBottom:
            startPoint = Lv2cPoint(0, 0);
            endPoint = Lv2cPoint(0, height);
            break;
        case Lv2cSlideAnimationType::SlideOutTop:
            startPoint = Lv2cPoint(0, 0);
            endPoint = Lv2cPoint(0, -height);
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

void Lv2cSlideInOutAnimationElement::OnLayoutComplete()
{
    if (this->pendingRequest)
    {
        this->pendingRequest = false;
        StartAnimation(animationType, animationLengthMs);
    }
}

void Lv2cSlideInOutAnimationElement::EasingFunction(Lv2cEasingFunction easingFunction) { animator.EasingFunction(easingFunction); }
void Lv2cSlideInOutAnimationElement::EasingFunction(std::function<double(double)> &&easingFunction) { animator.EasingFunction(std::move(easingFunction)); }
void Lv2cSlideInOutAnimationElement::BezierEasingFunction(Lv2cPoint p1, Lv2cPoint p2) { animator.BezierEasingFunction(p1, p2); }
