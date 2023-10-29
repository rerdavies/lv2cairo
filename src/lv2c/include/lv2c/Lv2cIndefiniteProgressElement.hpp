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
#include "Lv2cBindingProperty.hpp"
#include "Lv2cAnimator.hpp"

namespace lvtk {

    class Lv2cIndefiniteProgressElement : public Lv2cElement {
    public:
        using self = Lv2cIndefiniteProgressElement;
        using super = Lv2cElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cIndefiniteProgressElement();

        BINDING_PROPERTY(Active,bool,false);

    protected:
        virtual bool WillDraw() const override;
        virtual void OnDraw(Lv2cDrawingContext&dc) override;

        virtual void OnMount() override;
        virtual void OnUnmount() override;

        virtual void OnActiveChanged(bool value);
        virtual void OnLayoutComplete() override;
    private:
        static constexpr double RATE = 300; /*pixels per second*/

        void UpdateAnimationForLayout();
        double animationValue;
        void StartAnimation();
        void StopAnimation();

        Lv2cAnimator animator;

        double beadRatio = 0.001;
    };
}