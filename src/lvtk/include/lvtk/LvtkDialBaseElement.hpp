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
#include "LvtkSvgElement.hpp"
#include "LvtkBindingProperty.hpp"

namespace lvtk
{

    class LvtkDialBaseElement: public LvtkValueElement 
    {
    public:
        using super = LvtkValueElement;
        using self = LvtkDialBaseElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        LvtkDialBaseElement();

        LvtkDialBaseElement& Value(double value) { 
            ValueProperty.set(value); 
            return *this;
        }
        double Value() const {
            return ValueProperty.get();
        }
        LvtkBindingProperty<bool> IsDraggingProperty;

        BINDING_PROPERTY_REF(HoverOpacity,std::optional<LvtkHoverOpacity>,std::optional<LvtkHoverOpacity>())
    protected:
        virtual void OnHoverOpacityChanged(const std::optional<LvtkHoverOpacity>&value);
        virtual void OnHoverStateChanged(LvtkHoverState state) override;
        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseUp(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseMove(LvtkMouseEventArgs &event) override;

        virtual void OnMount() override;

        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override {
            return super::Measure(constraint,maxAvailable,context);
        }

        const LvtkHoverOpacity& GetHoverOpacity() const;

        void OnValueChanged(double value) override;
    private:
        virtual void OnDialOpacityChanged(double opacity);
        void UpdateMousePoint(LvtkMouseEventArgs &event);
        LvtkPoint lastMousePoint;
        double mouseValue;
    };

}