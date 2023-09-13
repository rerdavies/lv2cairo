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

#include "LvtkValueElement.hpp"
#include "LvtkBindingProperty.hpp"
#include <chrono>

namespace lvtk {
    class LvtkToggleThumbElement;
    class LvtkToggleTrackElement;
    class LvtkDropShadowElement;

    class LvtkSwitchElement : public LvtkValueElement
    {
    public:
        using self = LvtkSwitchElement;
        using super = LvtkValueElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }

        LvtkSwitchElement();

        LvtkSwitchElement&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() const { return ValueProperty.get(); }

        bool Checked() const { return Value() >= 0.5; }
        LvtkSwitchElement&Checked(bool value) { Value(value? 1.0: 0.0); return *this; }


    protected:
        virtual bool WillDraw() const override { return true; }
        virtual bool IsOnOff() const { return false; }
        virtual void OnValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
        virtual bool OnClick(LvtkMouseEventArgs &event);
        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseUp(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseMove(LvtkMouseEventArgs &event) override;
        virtual void OnHoverStateChanged(LvtkHoverState hoverState) override;

    private: 
        BINDING_PROPERTY(Position,double, 0.0);
        BINDING_PROPERTY(Pressed,bool,false);

        double secondsPerTick;

        double trackWidth = -1;
        using clock_t = std::chrono::steady_clock;
        void StartAnimation();
        void StopAnimation();
        void AnimationTick(const animation_clock_time_point_t&now);
        bool dragging = false;
        double startPosition = 0;
        LvtkPoint startPoint;

        AnimationHandle animationHandle;

        clock_t::time_point lastAnimationTime;
        bool checked = false;
        std::shared_ptr<LvtkToggleThumbElement> thumb;
        std::shared_ptr<LvtkToggleTrackElement> track;
        std::shared_ptr<LvtkDropShadowElement> thumbShadow;
        std::shared_ptr<LvtkDropShadowElement> trackShadow;

    };
}