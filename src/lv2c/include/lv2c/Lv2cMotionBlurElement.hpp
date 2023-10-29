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

namespace lvtk {
    class Lv2cSurface;

    class Lv2cMotionBlurElement : public Lv2cContainerElement {
    public:
        using self=Lv2cMotionBlurElement;
        using super=Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cMotionBlurElement();
        BINDING_PROPERTY(From,Lv2cPoint,Lv2cPoint(0,0));
        BINDING_PROPERTY(To,Lv2cPoint,Lv2cPoint(0,0));

        void Blur(Lv2cPoint from, Lv2cPoint to) { From(from); To(to); }        

    protected:
        bool WillDraw() const override;

        virtual void DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds) override;


        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context) override {
            super::Measure(constraint,maxAvailable,context);
        }
    private:

        Lv2cImageSurface MotionBlurFilter(Lv2cImageSurface& surface,Lv2cPoint from, Lv2cPoint to);

    };


}