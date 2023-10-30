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
#include "lv2c/Lv2cValueElement.hpp"
#include <chrono>

namespace lv2c
{
    enum class Lv2cLampVariant {
        // The lamp can take intermediate values.
        Blended,
        // The lamp is either on or off.
        OnOff
    };
    class Lv2cLampElement: public Lv2cValueElement {
    public:
        Lv2cLampElement();

        using self = Lv2cLampElement;
        using super = Lv2cValueElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        BINDING_PROPERTY(Variant,Lv2cLampVariant,Lv2cLampVariant::OnOff);
    protected:
        using clock_t = std::chrono::steady_clock;
        clock_t::time_point animationStartTime;
        double animationStartValue = 0;
        double animationTarget = 0;
        AnimationHandle animationHandle;

        void AnimationValue(double value);
        double AnimationValue() const;
        double animationValue = -1E8;
        void StartAnimation(double targetValue);
        void StopAnimation();
        void AnimationTick(const animation_clock_time_point_t&now);
        Lv2cElement::ptr image;
        virtual void OnVariantChanged(Lv2cLampVariant value);
        virtual void OnValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnDraw(Lv2cDrawingContext &dc) override;
    private:
        void UpdateLampColor();


    };
}
