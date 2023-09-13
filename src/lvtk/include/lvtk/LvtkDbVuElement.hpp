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
#include <chrono>
#include "LvtkVuElement.hpp"
namespace lvtk {
    class LvtkDbVuElement: public LvtkVuElement {
    public:
        using self = LvtkDbVuElement;
        using super = LvtkVuElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkDbVuElement();

        self&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() { return ValueProperty.get(); }

        self&MinValue(double value) { MinValueProperty.set(value); return *this; }
        double MinValue() { return MinValueProperty.get(); }

        self&MaxValue(double value) { MaxValueProperty.set(value); return *this; }
        double MaxValue() { return MaxValueProperty.get(); }

    protected:
        double ValueToClient(double value, const LvtkRectangle&vuRectangle);
        using clock_t = std::chrono::steady_clock;
        AnimationHandle animationHandle;
        clock_t::time_point animationStartTime;
        double animationStartValue = 0;
        void AnimationTick(const animation_clock_time_point_t&now);
        void StopAnimation();
        void StartAnimation(bool hold);
        BINDING_PROPERTY(HoldValue,double,0.0)
        virtual void OnValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void UpdateStyle() override;
        virtual const LvtkVuSettings &Settings() const override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
    private:
        friend class LvtkStereoDbVuElement;

        static void DrawTicks(
            LvtkDrawingContext &dc,
            double minValue, 
            double maxValue,
            const LvtkVuSettings&settings,
            const LvtkRectangle & vuRectangle,
            LvtkColor color);

        static void DrawTelltale(
            LvtkDrawingContext &dc, 
            double telltaleValue, 
            double value, 
            double minValue, 
            double maxValue, 
            const LvtkVuSettings&settings,
            const LvtkRectangle&vuBarRectangle);


    };

    class LvtkStereoDbVuElement: public LvtkStereoVuElement {
    public:
        using self = LvtkStereoDbVuElement;
        using super = LvtkStereoVuElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkStereoDbVuElement();

        self&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() { return ValueProperty.get(); }

        self&MinValue(double value) { MinValueProperty.set(value); return *this; }
        double MinValue() { return MinValueProperty.get(); }

        self&MaxValue(double value) { MaxValueProperty.set(value); return *this; }
        double MaxValue() { return MaxValueProperty.get(); }

    protected:
        using clock_t = std::chrono::steady_clock;
        AnimationHandle animationHandle;
        clock_t::time_point leftAnimationStartTime;
        clock_t::time_point rightAnimationStartTime;
        double leftAnimationStartValue = 0;
        double rightAnimationStartValue = 0;
        bool leftAnimationActive = false;
        bool rightAnimationActive = false;

        void AnimationTick(const animation_clock_time_point_t&now);
        void StopAnimation();
        void StartLeftAnimation(bool hold);
        void StartRightAnimation(bool hold);


        BINDING_PROPERTY(HoldValue,double,0.0)
        BINDING_PROPERTY(RightHoldValue,double,0.0)
        virtual void OnValueChanged(double value) override;
        virtual void OnRightValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void UpdateStyle() override;
        virtual const LvtkVuSettings &Settings() const override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;

    };

}