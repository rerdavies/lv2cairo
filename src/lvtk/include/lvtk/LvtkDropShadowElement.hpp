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

namespace lvtk
{

    class LvtkDropShadowElement : public LvtkContainerElement
    {
    public:
        using self = LvtkDropShadowElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkBindingProperty<LvtkDropShadowVariant> VariantProperty{LvtkDropShadowVariant::DropShadow};
        LvtkDropShadowVariant Variant() const { return VariantProperty.get(); }
        LvtkDropShadowElement &Variant(const LvtkDropShadowVariant &value)
        {
            VariantProperty.set(value);
            return *this;
        }

        LvtkBindingProperty<LvtkColor> ShadowColorProperty{LvtkColor(0, 0, 0)};
        const LvtkColor &ShadowColor() const { return ShadowColorProperty.get(); }
        LvtkDropShadowElement &ShadowColor(const LvtkColor &value)
        {
            ShadowColorProperty.set(value);
            return *this;
        }

        double Radius() const { return RadiusProperty.get(); }
        LvtkBindingProperty<double> RadiusProperty{8.0};
        LvtkDropShadowElement &Radius(double value)
        {
            RadiusProperty.set(value);
            return *this;
        }

        LvtkBindingProperty<double> XOffsetProperty{1.0};
        LvtkDropShadowElement &XOffset(double value)
        {
            XOffsetProperty.set(value);
            return *this;
        }
        double XOffset() const { return XOffsetProperty.get(); }

        LvtkBindingProperty<double> YOffsetProperty{2.0};
        LvtkDropShadowElement &YOffset(double value)
        {
            YOffsetProperty.set(value);
            return *this;
        }
        double YOffset() const { return YOffsetProperty.get(); }

        LvtkBindingProperty<double> ShadowOpacityProperty{1.0};
        LvtkDropShadowElement &ShadowOpacity(double value)
        {
            ShadowOpacityProperty.set(value);
            return *this;
        }
        double ShadowOpacity() const { return ShadowOpacityProperty.get(); }

        LvtkDropShadowElement &DropShadow(const LvtkDropShadow&dropShadow);
        LvtkDropShadow DropShadow() const;

    protected:
        virtual void InvalidateScreenRect(const LvtkRectangle &screenRectangle) override;
        virtual void FinalizeLayout(const LvtkRectangle &layoutClipRect, const LvtkRectangle &parentBounds, bool clippedInLayout) override;
    private:
        bool IsSolidBackground() const;
        bool IsInterior(const LvtkRectangle &rectangle) const;
        void BlurDropShadow(LvtkDrawingContext &dc, cairo_surface_t *surface, double *pXOffset, double *pYOffset);
        void BlurInsetDropShadow(LvtkDrawingContext &dc, cairo_surface_t *surface);
        void DrawDropShadow(LvtkDrawingContext &dc, const LvtkRectangle &clipRect);
        bool DrawFastDropShadow(LvtkDrawingContext &dc, const LvtkRectangle &clipRect);
        void DrawInsetDropShadow(LvtkDrawingContext &dc, const LvtkRectangle &clipRect);
        virtual void DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds) override;
    };

}