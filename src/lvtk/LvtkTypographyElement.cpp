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

#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/IcuString.hpp"

#include "pango/pangocairo.h"
#include "lvtk/LvtkPangoContext.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>

using namespace lvtk;

PangoEllipsizeMode ToPangoEllipsizeMode(LvtkEllipsizeMode cairoMode)
{
    return (PangoEllipsizeMode)(int)cairoMode;
}

using namespace lvtk;

bool LvtkTypographyElement::WillDraw() const
{
    return this->Text().length() != 0 || super::WillDraw();
}

void LvtkTypographyElement::OnVariantChanged(LvtkTypographyVariant value)
{
    hasDrawTextChanged = true;
    RemoveClass(variantStyle);
    variantStyle = GetVariantStyle();
    AddClass(variantStyle);
    InvalidateLayout();
}

void LvtkTypographyElement::OnTextChanged(const std::string &text)
{
    hasDrawTextChanged = true;
    Invalidate();
    if (!hasFixedLayout)
    {
        InvalidateLayout(); // todo: we could call Measure() to decide
    }
}

LvtkTypographyElement &LvtkTypographyElement::Text(const std::string &text)
{
    this->TextProperty.set(text);

    return *this;
}

const std::string &LvtkTypographyElement::Text() const { return TextProperty.get(); }

std::string LvtkTypographyElement::GetFontFamily()
{
    std::string fontFamily;
    fontFamily = Style().FontFamily();
    return gPangoContext.GetFontFamily(fontFamily);
}

LvtkTypographyElement::LvtkTypographyElement()
: icuString(IcuString::Instance())
{
    this->Style().HorizontalAlignment(LvtkAlignment::Start);
    TextProperty.SetElement(this, &LvtkTypographyElement::OnTextChanged);
    VariantProperty.SetElement(this, &LvtkTypographyElement::OnVariantChanged);
}

LvtkTypographyElement::~LvtkTypographyElement()
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

void OnMount(LvtkWindow *window)
{
}

LvtkSize LvtkTypographyElement::MeasureClient(LvtkSize constraint, LvtkSize available, LvtkDrawingContext &context)
{
    this->hasDrawTextChanged = false; // load text in OnDraw
    bool fixedWidth = constraint.Width() != 0;
    bool fixedHeight = constraint.Height() != 0;
    bool singleLine = SingleLine();
    if (pangoLayout == nullptr)
    {

        // pangoLayout = pango_cairo_create_layout(context.get());

        pangoLayout = pango_layout_new(GetPangoContext());
    }
    // ? pango_cairo_update_context ( context.get(),GetPangoContext());

    if (Style().TextTransform() == LvtkTextTransform::Capitalize)
    {
        uppercase = icuString->toUpper(Text());

        pango_layout_set_markup(pangoLayout, uppercase.c_str(), (int)(uppercase.length()));
    }
    else
    {
        pango_layout_set_markup(pangoLayout, this->Text().c_str(), (int)(this->Text().length()));
    }

    pango_layout_set_alignment(pangoLayout, (PangoAlignment)(int)Style().TextAlign());
    if (singleLine)
    {
        if (this->Text().length() == 0)
        {
            // pango returns spurious line heights if text is empty.
            pango_layout_set_markup(pangoLayout, "x", 1);
            this->hasDrawTextChanged = true;
        }

        pango_layout_set_width(pangoLayout, -1);
        pango_layout_set_height(pangoLayout, -1);
        pango_layout_set_ellipsize(pangoLayout, PangoEllipsizeMode::PANGO_ELLIPSIZE_NONE);
        pango_layout_set_alignment(pangoLayout, PangoAlignment::PANGO_ALIGN_LEFT);
    }
    else
    {
        double width = constraint.Width();
        if (width == 0)
        {
            width = available.Width();
        }

        pango_layout_set_ellipsize(pangoLayout, PangoEllipsizeMode::PANGO_ELLIPSIZE_NONE);
        pango_layout_set_width(pangoLayout, ((int)std::floor(width)) * PANGO_SCALE);
        pango_layout_set_line_spacing(pangoLayout,Style().LineSpacing());
        //pango_layout_set_height(pangoLayout, -50000); // max 50000 lines. That should be enough/
    }
    PangoFontDescription *desc = GetFontDescription();
    pango_layout_set_font_description(pangoLayout, desc);

    pango_cairo_update_layout(context.get(), pangoLayout);

    pango_font_description_free(desc);
    int x, y;
    pango_layout_get_size(pangoLayout, &x, &y);

    LvtkSize size = LvtkSize(std::ceil(x / PANGO_SCALE), std::ceil(y / PANGO_SCALE));

    this->textMeasure = size;
    if (singleLine && this->Text().length() == 0)
    {

        size.Width(0);
    }
    if (Style().Ellipsize() != LvtkEllipsizeMode::Disable)
    {
        if (available.Width() != 0 && available.Width() < size.Width())
        {
            size.Width(available.Width());
        }
        if (constraint.Width() != 0 && constraint.Width() < size.Width())
        {
            size.Width(constraint.Width());
        }
    }
    if (Style().HorizontalAlignment() == LvtkAlignment::Stretch && available.Width() != 0)
    {
        size.Width(available.Width());
        fixedWidth = true;
    }
    if (Style().VerticalAlignment() == LvtkAlignment::Stretch && available.Height() != 0)
    {
        size.Height(available.Height());
        fixedHeight = true;
    }
    if (available.Width() != 0 && size.Width() > available.Width())
    {
        size.Width(available.Width());
    }
    if (available.Height() != 0 && size.Height() > available.Height())
    {
        size.Height(available.Width());
    }
    if (constraint.Width() != 0)
    {
        size.Width(constraint.Width());
    }
    if (singleLine)
    {
        this->hasFixedLayout = fixedWidth;
    }
    else
    {
        this->hasFixedLayout = fixedWidth && fixedHeight;
    }
    return (size);
}
LvtkSize LvtkTypographyElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    LvtkSize borderSize = this->removeThickness(available, Style().Margin());
    LvtkSize paddingSize = this->removeThickness(borderSize, Style().BorderWidth());
    LvtkSize clientSize = this->removeThickness(paddingSize, Style().Padding());
    bool singleLine = SingleLine();
    if (singleLine)
    {
        if (clientSize.Width() < this->textMeasure.Width() - 1)
        {
            pango_layout_set_ellipsize(pangoLayout, ToPangoEllipsizeMode(Style().Ellipsize()));
            if (Style().Ellipsize() == LvtkEllipsizeMode::Center)
            {
                // bug in Ubuntu 22.04. Always off by 4.
                pango_layout_set_width(pangoLayout, ((int)std::floor(clientSize.Width()-4)) * PANGO_SCALE);

            } else {
                pango_layout_set_width(pangoLayout, ((int)std::floor(clientSize.Width()-4)) * PANGO_SCALE);
            }
        }
        else
        {
            pango_layout_set_width(pangoLayout, ((int)std::floor(clientSize.Width() + 5)) * PANGO_SCALE);
            pango_layout_set_ellipsize(pangoLayout, PangoEllipsizeMode::PANGO_ELLIPSIZE_NONE);
        }
        pango_layout_set_height(pangoLayout, -1);
    }
    else
    {
        pango_layout_set_line_spacing(pangoLayout,Style().LineSpacing());
        pango_layout_set_width(pangoLayout, ((int)std::floor(clientSize.Width())) * PANGO_SCALE);
    }

    PangoFontDescription *desc = GetFontDescription();

    pango_layout_set_font_description(pangoLayout, desc);

    pango_font_description_free(desc);
    desc = nullptr;

    pango_layout_set_alignment(pangoLayout, (PangoAlignment)(int)Style().TextAlign());

    pango_cairo_update_layout(context.get(), pangoLayout);

    int x, y;
    pango_layout_get_size(pangoLayout, &x, &y);

    PangoRectangle inkRect, logicalRect;
    pango_layout_get_extents(pangoLayout, &inkRect, &logicalRect);

    LvtkSize pangoSize = LvtkSize(std::ceil(x / PANGO_SCALE), std::ceil(y / PANGO_SCALE));

    LvtkSize size;
    if (singleLine)
    {
        size = pangoSize;
    }
    else
    {
        size = LvtkSize(clientSize.Width(), pangoSize.Height());
    }
    size = AddThickness(size, Style().Padding());
    size = AddThickness(size, Style().BorderWidth());
    size = AddThickness(size, Style().Margin());
    return size;
}

void LvtkTypographyElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    auto source = Style().Color();
    if (source)
    {
        cairo_save(dc.get());
        dc.set_source(source);
        if (hasDrawTextChanged)
        {
            hasDrawTextChanged = false;
            if (Style().TextTransform() == LvtkTextTransform::Capitalize)
            {
                this->uppercase = icuString->toUpper(Text());
                pango_layout_set_markup(pangoLayout, uppercase.c_str(), (int)(uppercase.length()));
            }
            else
            {
                pango_layout_set_markup(pangoLayout, this->Text().c_str(), (int)(this->Text().length()));
            }
            if (!SingleLine())
            {
                pango_layout_set_line_spacing(pangoLayout,Style().LineSpacing());
            }

            pango_cairo_update_layout(dc.get(), pangoLayout);
        }

        dc.move_to(0, 0);
        pango_cairo_show_layout(dc.get(), pangoLayout);
        cairo_restore(dc.get());
    }
}


LvtkStyle::ptr LvtkTypographyElement::GetVariantStyle()
{
    if (!this->IsMounted())
    {
        return nullptr;
    }
    const auto &theme = this->Theme();

    switch (Variant())
    {
    case LvtkTypographyVariant::Inherit:
        return nullptr;
    case LvtkTypographyVariant::Title:
        return theme.titleStyle;
    case LvtkTypographyVariant::BodySecondary:
        return theme.bodySecondaryStyle;
    case LvtkTypographyVariant::BodyPrimary:
        return theme.bodyPrimaryStyle;
    case LvtkTypographyVariant::Caption:
        return theme.captionStyle;
    case LvtkTypographyVariant::Heading:
        return theme.headingStyle;
    default:
        return nullptr;
    }
}

void LvtkTypographyElement::OnMount()
{
    super::OnMount();
    RemoveClass(variantStyle);
    variantStyle = GetVariantStyle();
    AddClass(variantStyle);
}

PangoFontDescription *LvtkTypographyElement::GetFontDescription()
{
    return gPangoContext.GetFontDescription(this->Style());
}

bool LvtkTypographyElement::SingleLine() const
{
    return Style().SingleLine();
}
LvtkEllipsizeMode LvtkTypographyElement::EllipsizeMode() const
{
    return Style().Ellipsize();
}

