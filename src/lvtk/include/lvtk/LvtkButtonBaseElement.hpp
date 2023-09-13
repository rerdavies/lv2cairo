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

#include "LvtkContainerElement.hpp"
#include "LvtkBindingProperty.hpp"
#include <chrono>


namespace lvtk {



    class LvtkButtonBaseElement: public LvtkContainerElement {
    public:
        virtual const char* Tag() const override { return "Button";}

        using self=LvtkButtonBaseElement;
        using super=LvtkContainerElement;
        using ptr =std::shared_ptr<LvtkButtonBaseElement>;
        static ptr Create() { return std::make_shared<LvtkButtonBaseElement>();}

        LvtkButtonBaseElement();

        LvtkEvent<LvtkMouseEventArgs> Clicked;
        
        BINDING_PROPERTY(Enabled,bool,true)

    protected:
        virtual const LvtkHoverColors &HoverBackgroundColors();
        virtual void OnEnabledChanged(bool value);
        virtual bool ShowPressedState() const;

    protected:
        virtual bool OnFocus(const LvtkFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const LvtkFocusEventArgs &eventArgs) override;

        virtual void OnHoverStateChanged(LvtkHoverState state) override;
        virtual bool OnClick(LvtkMouseEventArgs &event);

        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseMove(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseUp(LvtkMouseEventArgs &event) override;


        virtual void OnDraw(LvtkDrawingContext &dc) override;


        virtual void OnMount() override;
        virtual void OnUnmount() override;

        virtual bool WillDraw() const override { return true; }

    private:

        using clock_t = std::chrono::steady_clock;
        clock_t::time_point animationStartTime; 
        double animationStartValue = 0;
        bool animationIncreasing = false;
        AnimationHandle animationHandle;

        double animationValue = 0;
        LvtkPoint animationCenter;
        void AnimationValue(double value);
        void RequestAnimationTick();
        void AnimationTick(const animation_clock_time_point_t&now);
        void CancelAnimation();
        void StartAnimation(bool increasing);
        AnimationHandle clickEventHandle;
        LvtkHoverColors hoverColors;
    };
} // namespace