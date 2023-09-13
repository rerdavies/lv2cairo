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

#pragma once

#include "LvtkElement.hpp"

#include <chrono>
#include <functional>
namespace lvtk
{

    enum class LvtkEasingFunction {
        Linear,
        EaseIn,
        EaseOut,
        EaseInQuad,
        EaseInQuint
    };

    class LvtkAnimator
    {
    public:
        using clock_t = std::chrono::steady_clock;
        using easing_function_t = std::function<double(double)>;

        ~LvtkAnimator();
        void Initialize(
            LvtkElement *owner,
            clock_t::duration timeIn,
            clock_t::duration timeout,
            std::function<void(double value)> &&callback,
            double initialValue = 0.0);
        void Initialize(
            LvtkElement *owner,
            double timeInSeconds,
            double timeoutSeconds,
            std::function<void(double value)> &&callback,
            double initialValue = 0.0);

        template <typename Rep, typename Period>
        void Initialize(
            LvtkElement *owner,
            std::chrono::duration<Rep, Period> timeIn,
            std::chrono::duration<Rep, Period> timeOut,
            std::function<void(double value)> &&callback,
            double initialValue = 0.0);

        void SetTarget(double value);
        void Animate(double from, double to);

        void Animate(double from, double to, double timeInSeconds);
        void Animate(double from, double to, clock_t::duration time);
        void CancelAnimation();

        double Value() const { return currentValue; }

        void EasingFunction(LvtkEasingFunction easingFunction);
        void EasingFunction(easing_function_t &&function);

        void BezierEasingFunction(LvtkPoint p1, LvtkPoint p2);

    private:
        bool ownerMounted = false;
        easing_function_t easingFunction = [] (double v) { return v;};
        void OnOwnerMounted(LvtkWindow *window);
        void OnOwnerUnmounted(LvtkWindow *window);

        void AnimationTick(clock_t::time_point now);
        void StartAnimation();
        void StopAnimation();
        AnimationHandle animationHandle;
        double currentValue = 0;
        double targetValue = 0;
        LvtkElement *owner = nullptr;
        double tickRateIn;
        double tickRateOut;
        std::function<void(double)> callback;

        clock_t::time_point lastAnimationTime;
    };

    ///////////////////

    template <typename Rep, typename Period>
    void LvtkAnimator::Initialize(
        LvtkElement *owner,
        std::chrono::duration<Rep, Period> timeIn,
        std::chrono::duration<Rep, Period> timeOut,
        std::function<void(double value)> &&callback,
        double initialValue)

    {
        using namespace std::chrono;
        Initialize(
            owner,
            duration_cast<clock_t::duration>(timeIn),
            duration_cast<clock_t::duration>(timeOut),
            std::move(callback),
            initialValue);
    }
}