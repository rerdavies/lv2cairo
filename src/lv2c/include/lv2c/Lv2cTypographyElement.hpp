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
#include <set>
#include <string>
#include "Lv2cBindingProperty.hpp"
#include "IcuString.hpp"

typedef struct _PangoLayout PangoLayout;
typedef struct _PangoFontDescriptor PangoFontDescriptor;
typedef struct _PangoFontMap PangoFontMap;
typedef struct _PangoFontDescription PangoFontDescription;

namespace lv2c
{

    class Lv2cPangoContext;

    
    const std::set<std::string> &FontFamilies();

    enum class Lv2cTypographyVariant {
        Title,
        Heading,
        Subheading,
        BodyPrimary,
        BodySecondary,
        Caption,
        Inherit
    };

    class Lv2cTypographyElement : public Lv2cElement
    {
    public:
        virtual const char* Tag() const override { return "Typography";}

        using self = Lv2cTypographyElement;
        using super=Lv2cElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        using text_property_t = Lv2cBindingProperty<std::string>;

        Lv2cTypographyElement();
        virtual ~Lv2cTypographyElement() override;


        BINDING_PROPERTY(Variant,Lv2cTypographyVariant,Lv2cTypographyVariant::BodySecondary)

        Lv2cBindingProperty<std::string> TextProperty;
        Lv2cTypographyElement &Text(const std::string &text);
        const std::string &Text() const;

        
        virtual bool WillDraw() const override;

    public:
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;

    protected:
        void OnTextChanged(const std::string&text);
        void OnVariantChanged(Lv2cTypographyVariant value);
        void OnMount() override;
    private:
        IcuString::Ptr icuString;
        bool SingleLine() const;
        Lv2cEllipsizeMode EllipsizeMode() const;


        std::string uppercase;
        bool hasDrawTextChanged = true;
        bool hasFixedLayout = false;
        Lv2cSize clientMeasure;
        Lv2cStyle::ptr GetVariantStyle();

        PangoFontDescription*GetFontDescription();

        virtual Lv2cSize MeasureClient(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override;

        virtual void OnDraw(Lv2cDrawingContext &dc) override;
        PangoLayout *pangoLayout = nullptr;
        PangoFontDescriptor *fontDescriptor = nullptr;
        std::string GetFontFamily();

        Lv2cSize textMeasure;

        Lv2cStyle::ptr variantStyle;

        observer_handle_t textObserverHandle;
    };

} // namespace