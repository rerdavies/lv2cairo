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

#pragma once 

#include "Lv2cMotionBlurElement.hpp"
#include "Lv2cAnimator.hpp"


namespace lvtk {
    enum class Lv2cSlideAnimationType {
        None,
        SlideInStart,
        SlideInEnd,
        SlideInTop,
        SlideInBottom,
        SlideOutStart,
        SlideOutEnd,
        SlideOutBottom,
        SlideOutTop
    };
    class Lv2cSlideInOutAnimationElement : public Lv2cMotionBlurElement {
    public:
        using self = Lv2cSlideInOutAnimationElement;
        using super = Lv2cMotionBlurElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cSlideInOutAnimationElement();
        void StartAnimation(Lv2cSlideAnimationType type, double milliseconds);

        void EasingFunction(Lv2cEasingFunction easingFunction);
        void EasingFunction(std::function<double(double)> &&easingFunction);
        void BezierEasingFunction(Lv2cPoint p1, Lv2cPoint p2);
    protected:
        virtual void OnLayoutComplete() override;
    private:
        
        void AnimationTick(double value);
        bool pendingRequest = false;
        Lv2cSlideAnimationType animationType = Lv2cSlideAnimationType::None;
        double animationLengthMs;


        Lv2cPoint startPoint,endPoint;

        Lv2cPoint lastPoint;
        Lv2cAnimator animator;

    };
}