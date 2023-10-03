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

#include "LvtkMotionBlurElement.hpp"
#include "LvtkAnimator.hpp"


namespace lvtk {
    enum class LvtkSlideAnimationType {
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
    class LvtkSlideInOutAnimationElement : public LvtkMotionBlurElement {
    public:
        using self = LvtkSlideInOutAnimationElement;
        using super = LvtkMotionBlurElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkSlideInOutAnimationElement();
        void StartAnimation(LvtkSlideAnimationType type, double milliseconds);

        void EasingFunction(LvtkEasingFunction easingFunction);
        void EasingFunction(std::function<double(double)> &&easingFunction);
        void BezierEasingFunction(LvtkPoint p1, LvtkPoint p2);
    protected:
        virtual void OnLayoutComplete() override;
    private:
        
        void AnimationTick(double value);
        bool pendingRequest = false;
        LvtkSlideAnimationType animationType = LvtkSlideAnimationType::None;
        double animationLengthMs;


        LvtkPoint startPoint,endPoint;

        LvtkPoint lastPoint;
        LvtkAnimator animator;

    };
}