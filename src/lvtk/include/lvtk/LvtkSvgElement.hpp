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

#include "LvtkElement.hpp"
#include "LvtkBindingProperty.hpp"

namespace lvtk
{
    class LvtkSvg;

    class LvtkSvgElement : public LvtkElement
    {
    public:
        using super = LvtkElement;
        using ptr = std::shared_ptr<LvtkSvgElement>;

        static ptr Create() { return std::make_shared<LvtkSvgElement>(); }

        LvtkSvgElement();

        LvtkBindingProperty<std::string> SourceProperty;
        LvtkSvgElement &Source(const std::string &source);
        const std::string &Source() const;

        LvtkBindingProperty<double> RotationProperty;
        LvtkSvgElement &Rotation(double angle);
        double Rotation() const;

    protected:
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context) override;

    private:
        void OnRotationChanged(double value);
        void OnSourceChanged(const std::string&value);
        
        virtual bool WillDraw() const override { return true; }
        LvtkSize measuredImageSize;
        void Load();
        void OnDraw(LvtkDrawingContext &dc) override;
        void OnMount() override;

        bool changed = false;
        std::shared_ptr<LvtkSvg> image;
        Observable<double>::handle_t rotationObserverHandle;
        Observable<std::string>::handle_t sourceObserverHandle;
    };

}