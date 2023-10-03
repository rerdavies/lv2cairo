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

#include "LvtkContainerElement.hpp"

#include "LvtkBindingProperty.hpp"

namespace lvtk {
    class LvtkSurface;

    class LvtkMotionBlurElement : public LvtkContainerElement {
    public:
        using self=LvtkMotionBlurElement;
        using super=LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkMotionBlurElement();
        BINDING_PROPERTY(From,LvtkPoint,LvtkPoint(0,0));
        BINDING_PROPERTY(To,LvtkPoint,LvtkPoint(0,0));

        void Blur(LvtkPoint from, LvtkPoint to) { From(from); To(to); }        

    protected:
        bool WillDraw() const override;

        virtual void DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds) override;


        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable, LvtkDrawingContext &context) override {
            super::Measure(constraint,maxAvailable,context);
        }
    private:

        LvtkImageSurface MotionBlurFilter(LvtkImageSurface& surface,LvtkPoint from, LvtkPoint to);

    };


}