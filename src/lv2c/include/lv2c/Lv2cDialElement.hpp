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

#include "Lv2cDialBaseElement.hpp"
#include "Lv2cBindingProperty.hpp"
#include "Lv2cSvgElement.hpp"
#include "Lv2cDropShadowElement.hpp"
#include "Lv2cBindingProperty.hpp"

namespace lvtk
{

    /// @brief A dial control element.
    /// The value of the dial is bound to ValueProperty. 
    ///
    /// The Source property is a path to the svg file used to render the dial. By default, it is
    // "fx_dial.svg".
    ///
    /// By default, the SVG is drawn in a solid color (the alpha-channel of the rendered 
    /// SVG, filled with the Color style attribute). This behaviour can be disabled 
    /// by setting the TintImage property to false.
    ///
    /// The dial is drawn with an optional drop-shadow effect (by default an Inset drop-shadow). 
    /// this can be disabled by setting the DropShadowVariant property to to Lv2cDropShadowVariant::Empty.
    ///

    class Lv2cDialElement: public Lv2cDialBaseElement 
    {
    public:
        using super = Lv2cDialBaseElement;
        using self = Lv2cDialElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        Lv2cDialElement();

        BINDING_PROPERTY_REF(Source,std::string,std::string("fx_dial.svg"))

        BINDING_PROPERTY_REF(DropShadow,std::optional<Lv2cDropShadow>,std::optional<Lv2cDropShadow>())

        BINDING_PROPERTY(TintImage,bool,true)

        Lv2cDialElement& Value(double value) { 
            ValueProperty.set(value); 
            return *this;
        }
        double Value() const {
            return ValueProperty.get();
        }
    protected:
        virtual void OnDialOpacityChanged(double opacity) override;

        virtual void OnMount() override;
        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override {
            return super::Measure(constraint,maxAvailable,context);
        }


        virtual void OnValueChanged(double value) override;
    private:
        Lv2cDropShadowElement::ptr dropShadow;
        Lv2cSvgElement::ptr image;
    };

}