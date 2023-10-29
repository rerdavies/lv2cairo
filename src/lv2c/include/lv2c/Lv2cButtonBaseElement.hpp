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

#include "Lv2cContainerElement.hpp"
#include "Lv2cBindingProperty.hpp"
#include <chrono>


namespace lvtk {



    class Lv2cButtonBaseElement: public Lv2cContainerElement {
    public:
        virtual const char* Tag() const override { return "Button";}

        using self=Lv2cButtonBaseElement;
        using super=Lv2cContainerElement;
        using ptr =std::shared_ptr<Lv2cButtonBaseElement>;
        static ptr Create() { return std::make_shared<Lv2cButtonBaseElement>();}

        Lv2cButtonBaseElement();

        Lv2cEvent<Lv2cMouseEventArgs> Clicked;
        
        BINDING_PROPERTY(Enabled,bool,true)

    protected:
        virtual const Lv2cHoverColors &HoverBackgroundColors();
        virtual void OnEnabledChanged(bool value);
        virtual bool ShowPressedState() const;

        void FireKeyboardClick();
    protected:
        Lv2cMouseEventArgs MakeKeyboardEventArgs();

        virtual bool OnFocus(const Lv2cFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const Lv2cFocusEventArgs &eventArgs) override;

        virtual void OnHoverStateChanged(Lv2cHoverState state) override;
        virtual bool OnClick(Lv2cMouseEventArgs &event);

        virtual bool OnMouseDown(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseMove(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event) override;


        virtual void OnDraw(Lv2cDrawingContext &dc) override;


        virtual void OnMount() override;
        virtual void OnUnmount() override;

        virtual bool WillDraw() const override { return true; }

    private:
        AnimationHandle keyboardDelayHandle;
        void CancelKeyboardDelay();


        using clock_t = std::chrono::steady_clock;
        clock_t::time_point animationStartTime; 
        double animationStartValue = 0;
        bool animationIncreasing = false;
        AnimationHandle animationHandle;

        double animationValue = 0;
        Lv2cPoint animationCenter;
        void AnimationValue(double value);
        void RequestAnimationTick();
        void AnimationTick(const animation_clock_time_point_t&now);
        void CancelAnimation();
        void StartAnimation(bool increasing);
        AnimationHandle clickEventHandle;
        Lv2cHoverColors hoverColors;
    };
} // namespace