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

namespace lv2c
{
    class Lv2cSvg;

    class Lv2cSvgElement : public Lv2cElement
    {
    public:
        using super = Lv2cElement;
        using ptr = std::shared_ptr<Lv2cSvgElement>;

        static ptr Create() { return std::make_shared<Lv2cSvgElement>(); }

        Lv2cSvgElement();

        Lv2cBindingProperty<std::string> SourceProperty;
        Lv2cSvgElement &Source(const std::string &source);
        const std::string &Source() const;

        Lv2cBindingProperty<double> RotationProperty;
        Lv2cSvgElement &Rotation(double angle);
        double Rotation() const;

    protected:
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context) override;

    private:
        void OnRotationChanged(double value);
        void OnSourceChanged(const std::string&value);
        
        virtual bool WillDraw() const override { return true; }
        Lv2cSize measuredImageSize;
        void Load();
        void OnDraw(Lv2cDrawingContext &dc) override;
        void OnMount() override;

        bool changed = false;
        std::shared_ptr<Lv2cSvg> image;
        Observable<double>::handle_t rotationObserverHandle;
        Observable<std::string>::handle_t sourceObserverHandle;
    };

}