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

#include "LvtkDialBaseElement.hpp"
#include "LvtkBindingProperty.hpp"
#include "LvtkDropShadowElement.hpp"
#include "LvtkBindingProperty.hpp"
#include "LvtkPngStripElement.hpp"

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
    /// this can be disabled by setting the DropShadowVariant property to to LvtkDropShadowVariant::Empty.
    ///

    class LvtkPngDialElement: public LvtkDialBaseElement 
    {
    public:
        using super = LvtkDialBaseElement;
        using self = LvtkPngDialElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        LvtkPngDialElement();

        BINDING_PROPERTY_REF(Source,std::string,std::string("dial_strip.svg"))

        LvtkPngDialElement& Value(double value) { 
            ValueProperty.set(value); 
            return *this;
        }
        double Value() const {
            return ValueProperty.get();
        }
    protected:
        virtual void OnDialOpacityChanged(double opacity) override;

        virtual void OnMount() override;
        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override {
            return super::Measure(constraint,maxAvailable,context);
        }
    private:
        LvtkPngStripElement::ptr image;
    };

}