// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
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

#include "lv2c/Lv2cButtonElement.hpp"


namespace lv2c::ui {

    class Lv2MomentaryButtonElement: public Lv2cButtonElement {
    public:

        enum class ButtonType {
            Trigger,    // one shot, sends max value on mouse down, nothing on mouse up.
            Momentary,  // Sends max value on mouse down, min value on mouse up.
            MomentaryOnByDefault // sends min value on mouse down, max value on mouse up.
        };
        virtual const char* Tag() const override { return "MomentaryButton";}

        using self = Lv2MomentaryButtonElement;
        using super=Lv2cButtonElement;
        using ptr =std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>();}

        Lv2MomentaryButtonElement() { 
            Variant(Lv2cButtonVariant::FilledButtonPrimary);
        };

        Lv2cBindingProperty<double> ValueProperty;
        self&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() { return ValueProperty.get(); }

        self&MinValue(double value) { minValue = value; return *this; }
        double MinValue() const { return minValue; }

        self&MaxValue(double value) { maxValue = value; return *this; }
        double MaxValue() const { return maxValue; }

        self&SetButtonType(ButtonType value) { buttonType = value; return *this; }
        ButtonType GetButtonType() const { return buttonType; }
    protected: 
        virtual bool OnMouseDown(Lv2cMouseEventArgs &event) override
        {
            super::OnMouseDown(event);
            Value(buttonType == ButtonType::MomentaryOnByDefault ? minValue: maxValue);
            return true;
        }
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event) override
        {
            super::OnMouseUp(event);
            switch (buttonType) 
            {
                case ButtonType::Momentary:
                    Value(minValue);
                    break;
                case ButtonType::MomentaryOnByDefault:
                    Value(maxValue);
                    break;
                case ButtonType::Trigger:
                    // Should auto-reset, but Reaper doesn't implement that.
                    Value(minValue);
                    break;
            }
            return true;
        }

    private:
        ButtonType buttonType = ButtonType::Trigger;
        double minValue = 0;
        double maxValue = 1;
    };
} // namespace