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
#include "lvtk/LvtkValueElement.hpp"
#include <chrono>

namespace lvtk
{
    enum class LvtkLampVariant {
        // The lamp can take intermediate values.
        Blended,
        // The lamp is either on or off.
        OnOff
    };
    class LvtkLampElement: public LvtkValueElement {
    public:
        LvtkLampElement();

        using self = LvtkLampElement;
        using super = LvtkValueElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        BINDING_PROPERTY(Variant,LvtkLampVariant,LvtkLampVariant::OnOff);
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
        LvtkElement::ptr image;
        virtual void OnVariantChanged(LvtkLampVariant value);
        virtual void OnValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
    private:
        void UpdateLampColor();


    };
}
