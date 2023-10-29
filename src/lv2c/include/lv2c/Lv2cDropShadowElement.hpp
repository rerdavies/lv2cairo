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

namespace lvtk
{

    class Lv2cDropShadowElement : public Lv2cContainerElement
    {
    public:
        using self = Lv2cDropShadowElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cBindingProperty<Lv2cDropShadowVariant> VariantProperty{Lv2cDropShadowVariant::DropShadow};
        Lv2cDropShadowVariant Variant() const { return VariantProperty.get(); }
        Lv2cDropShadowElement &Variant(const Lv2cDropShadowVariant &value)
        {
            VariantProperty.set(value);
            return *this;
        }

        Lv2cBindingProperty<Lv2cColor> ShadowColorProperty{Lv2cColor(0, 0, 0)};
        const Lv2cColor &ShadowColor() const { return ShadowColorProperty.get(); }
        Lv2cDropShadowElement &ShadowColor(const Lv2cColor &value)
        {
            ShadowColorProperty.set(value);
            return *this;
        }

        double Radius() const { return RadiusProperty.get(); }
        Lv2cBindingProperty<double> RadiusProperty{8.0};
        Lv2cDropShadowElement &Radius(double value)
        {
            RadiusProperty.set(value);
            return *this;
        }

        Lv2cBindingProperty<double> XOffsetProperty{1.0};
        Lv2cDropShadowElement &XOffset(double value)
        {
            XOffsetProperty.set(value);
            return *this;
        }
        double XOffset() const { return XOffsetProperty.get(); }

        Lv2cBindingProperty<double> YOffsetProperty{2.0};
        Lv2cDropShadowElement &YOffset(double value)
        {
            YOffsetProperty.set(value);
            return *this;
        }
        double YOffset() const { return YOffsetProperty.get(); }

        Lv2cBindingProperty<double> ShadowOpacityProperty{1.0};
        Lv2cDropShadowElement &ShadowOpacity(double value)
        {
            ShadowOpacityProperty.set(value);
            return *this;
        }
        double ShadowOpacity() const { return ShadowOpacityProperty.get(); }

        Lv2cDropShadowElement &DropShadow(const Lv2cDropShadow&dropShadow);
        Lv2cDropShadow DropShadow() const;

    protected:
        virtual void InvalidateScreenRect(const Lv2cRectangle &screenRectangle) override;
        virtual void FinalizeLayout(const Lv2cRectangle &layoutClipRect, const Lv2cRectangle &parentBounds, bool clippedInLayout) override;
    private:
        bool IsSolidBackground() const;
        bool IsInterior(const Lv2cRectangle &rectangle) const;
        void BlurDropShadow(Lv2cDrawingContext &dc, cairo_surface_t *surface, double *pXOffset, double *pYOffset);
        void BlurInsetDropShadow(Lv2cDrawingContext &dc, cairo_surface_t *surface);
        void DrawDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipRect);
        bool DrawFastDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipRect);
        void DrawInsetDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipRect);
        virtual void DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds) override;
    };

}