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

namespace lv2c
{
    class Lv2cProgressElement : public Lv2cValueElement
    {
    public:
        using self = Lv2cProgressElement;
        using super = Lv2cValueElement;
        using ptr = std::shared_ptr<self>;
        virtual const char *Tag() const override { return "Lv2cProgressElement"; }
        static ptr Create() { return std::make_shared<self>(); }

    public:
        self&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() { return ValueProperty.get(); }

        BINDING_PROPERTY(MaxValue, double, 1.0)
        BINDING_PROPERTY(MinValue, double, 1.0)
    protected:
        virtual void OnValueChanged(double value) override;
        virtual void UpdateStyle();
        virtual const Lv2cVuSettings &Settings() const;
        virtual void OnMount() override;
        bool WillDraw() const override { return true; }
        virtual void OnDraw(Lv2cDrawingContext &dc) override;
    private:
        friend class Lv2cStereoVuElement;
        friend class Lv2cDbVuElement;
        friend class Lv2cStereoDbVuElement;
        
        static double ValueToClient(double value, double minValue, double maxValue,const Lv2cRectangle &vuRectangle);
        static void DrawVu(
            Lv2cDrawingContext &dc, 
            double value,
            double minValue, 
            double maxValue,
            const Lv2cRectangle&vuRectangle,
            const Lv2cVuSettings &settings);

        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context) override
        {
            super::Measure(constraint,maxAvailable,context);
        }

    };

}