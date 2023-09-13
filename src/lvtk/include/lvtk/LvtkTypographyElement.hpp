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
#include "LvtkElement.hpp"
#include <set>
#include <string>
#include "LvtkBindingProperty.hpp"
#include "IcuString.hpp"

typedef struct _PangoLayout PangoLayout;
typedef struct _PangoFontDescriptor PangoFontDescriptor;
typedef struct _PangoFontMap PangoFontMap;
typedef struct _PangoFontDescription PangoFontDescription;

namespace lvtk
{

    class LvtkPangoContext;

    
    const std::set<std::string> &FontFamilies();

    enum class LvtkTypographyVariant {
        Title,
        Heading,
        BodyPrimary,
        BodySecondary,
        Caption,
        Inherit
    };

    class LvtkTypographyElement : public LvtkElement
    {
    public:
        virtual const char* Tag() const override { return "Typography";}

        using self = LvtkTypographyElement;
        using super=LvtkElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        using text_property_t = LvtkBindingProperty<std::string>;

        LvtkTypographyElement();
        virtual ~LvtkTypographyElement() override;


        BINDING_PROPERTY(Variant,LvtkTypographyVariant,LvtkTypographyVariant::BodySecondary)

        LvtkBindingProperty<std::string> TextProperty;
        LvtkTypographyElement &Text(const std::string &text);
        const std::string &Text() const;

        
        virtual bool WillDraw() const override;

    public:
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;

    protected:
        void OnTextChanged(const std::string&text);
        void OnVariantChanged(LvtkTypographyVariant value);
    private:
        IcuString::Ptr icuString;
        bool SingleLine() const;
        LvtkEllipsizeMode EllipsizeMode() const;


        std::string uppercase;
        bool hasDrawTextChanged = true;
        bool hasFixedLayout = false;
        LvtkSize clientMeasure;
        LvtkStyle::ptr GetVariantStyle();
        void OnMount(LvtkWindow*window) override;

        PangoFontDescription*GetFontDescription();

        virtual LvtkSize MeasureClient(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override;

        virtual void OnDraw(LvtkDrawingContext &dc) override;
        PangoLayout *pangoLayout = nullptr;
        PangoFontDescriptor *fontDescriptor = nullptr;
        std::string GetFontFamily();

        LvtkSize textMeasure;

        LvtkStyle::ptr variantStyle;

        observer_handle_t textObserverHandle;
    };

} // namespace