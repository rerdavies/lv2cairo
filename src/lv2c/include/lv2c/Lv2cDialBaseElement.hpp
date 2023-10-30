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

#include "Lv2cValueElement.hpp"
#include "Lv2cBindingProperty.hpp"
#include "Lv2cSvgElement.hpp"
#include "Lv2cBindingProperty.hpp"

namespace lv2c
{

    class Lv2cDialBaseElement: public Lv2cValueElement 
    {
    public:
        using super = Lv2cValueElement;
        using self = Lv2cDialBaseElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        Lv2cDialBaseElement();

        Lv2cDialBaseElement& Value(double value) { 
            ValueProperty.set(value); 
            return *this;
        }
        double Value() const {
            return ValueProperty.get();
        }
        Lv2cBindingProperty<bool> IsDraggingProperty;

        BINDING_PROPERTY_REF(HoverOpacity,std::optional<Lv2cHoverOpacity>,std::optional<Lv2cHoverOpacity>())
    protected:
        virtual void OnHoverOpacityChanged(const std::optional<Lv2cHoverOpacity>&value);
        virtual void OnHoverStateChanged(Lv2cHoverState state) override;
        virtual bool OnMouseDown(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseMove(Lv2cMouseEventArgs &event) override;

        virtual void OnMount() override;

        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override {
            return super::Measure(constraint,maxAvailable,context);
        }

        const Lv2cHoverOpacity& GetHoverOpacity() const;

        void OnValueChanged(double value) override;
    private:
        virtual void OnDialOpacityChanged(double opacity);
        void UpdateMousePoint(Lv2cMouseEventArgs &event);
        Lv2cPoint lastMousePoint;
        double mouseValue;
    };

}