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

#include "Lv2cElement.hpp"

#include <chrono>
#include <functional>
namespace lv2c
{

    enum class Lv2cEasingFunction {
        Linear,
        EaseIn,
        EaseOut,
        EaseInQuad,
        EaseInQuint
    };

    class Lv2cAnimator
    {
    public:
        using clock_t = std::chrono::steady_clock;
        using easing_function_t = std::function<double(double)>;

        ~Lv2cAnimator();
        void Initialize(
            Lv2cElement *owner,
            clock_t::duration timeIn,
            clock_t::duration timeout,
            std::function<void(double value)> &&callback,
            double initialValue = 0.0);
        void Initialize(
            Lv2cElement *owner,
            double timeInSeconds,
            double timeoutSeconds,
            std::function<void(double value)> &&callback,
            double initialValue = 0.0);

        template <typename Rep, typename Period>
        void Initialize(
            Lv2cElement *owner,
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

        void EasingFunction(Lv2cEasingFunction easingFunction);
        void EasingFunction(easing_function_t &&function);

        void BezierEasingFunction(Lv2cPoint p1, Lv2cPoint p2);

    private:
        bool ownerMounted = false;
        easing_function_t easingFunction = [] (double v) { return v;};
        void OnOwnerMounted(Lv2cWindow *window);
        void OnOwnerUnmounted(Lv2cWindow *window);

        void AnimationTick(clock_t::time_point now);
        void StartAnimation();
        void StopAnimation();
        AnimationHandle animationHandle;
        double currentValue = 0;
        double targetValue = 0;
        Lv2cElement *owner = nullptr;
        double tickRateIn;
        double tickRateOut;
        std::function<void(double)> callback;

        clock_t::time_point lastAnimationTime;
    };

    ///////////////////

    template <typename Rep, typename Period>
    void Lv2cAnimator::Initialize(
        Lv2cElement *owner,
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