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

#include "lv2c/Lv2cStatusTextElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cPangoContext.hpp"
#include "lv2c/Lv2cDropdownItemElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "pango/pangocairo.h"
#include "lv2c/Lv2cPangoContext.hpp"

#include <chrono>


using namespace lvtk;


Lv2cSize Lv2cStatusTextElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context)
{
    if (clientConstraint.Width() != 0)
    {
        // fixed width: use standard measure.
        return super::MeasureClient(clientConstraint,clientAvailable,context);
    } else {

        if (pangoLayout == nullptr){
            pangoLayout = pango_layout_new(GetPangoContext());
        }
        PangoFontDescription *desc = gPangoContext.GetFontDescription(Style());

        pango_layout_set_font_description(pangoLayout, desc);

        double maxWidth = 20; 
        for (auto &dropdownItem: this->DropdownItems())
        {
            pango_layout_set_text(pangoLayout,dropdownItem.Text().c_str(),dropdownItem.Text().length());
            pango_cairo_update_layout(context.get(), pangoLayout);

            PangoRectangle pangoExtent;
            pango_layout_get_extents(pangoLayout,nullptr,&pangoExtent);
            double width = std::ceil(pangoExtent.width/PANGO_SCALE);
            if (width > maxWidth) maxWidth = width;
        }
        maxWidth += 4; // for good luck.

        pango_font_description_free(desc);

        clientConstraint.Width(maxWidth);

        return super::MeasureClient(clientConstraint,clientAvailable,context);
    }
}
Lv2cStatusTextElement::~Lv2cStatusTextElement() noexcept
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

Lv2cStatusTextElement::Lv2cStatusTextElement()
{
    SelectedId(INVALID_SELECTION_ID);
    typography = Lv2cTypographyElement::Create();
    typography->Variant(Lv2cTypographyVariant::BodySecondary);
    this->AddChild(typography);

    typography->Style().HorizontalAlignment(Lv2cAlignment::Stretch)
        .SingleLine(true)
        .Ellipsize(Lv2cEllipsizeMode::End);

    TextProperty.Bind(typography->TextProperty);

    UnselectedTextProperty.SetElement(this,&Lv2cStatusTextElement::OnUnselectedTextChanged);
    SelectedIdProperty.SetElement(this,&Lv2cStatusTextElement::OnSelectedIdChanged);
    DropdownItemsProperty.SetElement(this,&Lv2cStatusTextElement::OnDropdownItemsChanged);

}

const std::string &Lv2cStatusTextElement::Text() const
{

    return TextProperty.get();
}
Lv2cStatusTextElement &Lv2cStatusTextElement::Text(const std::string &text)
{
    TextProperty.set(text);
    return *this;
}

void Lv2cStatusTextElement::OnMount()
{
    auto &theme = Theme();
    this->AddClass(theme.statusTextStyle);
    UpdateText();
}





selection_id_t Lv2cStatusTextElement::SelectedId() const
{
    return SelectedIdProperty.get();
}
Lv2cStatusTextElement &Lv2cStatusTextElement::SelectedId(selection_id_t selectedId)
{
    SelectedIdProperty.set(selectedId);
    return *this;
}


const std::string &Lv2cStatusTextElement::UnselectedText() const
{
    return UnselectedTextProperty.get();
}
Lv2cStatusTextElement &Lv2cStatusTextElement::UnselectedText(const std::string &text)
{
    UnselectedTextProperty.set(text);
    return *this;
}


void Lv2cStatusTextElement::OnSelectedIdChanged(selection_id_t value)
{
    UpdateText();
}
void Lv2cStatusTextElement::OnUnselectedTextChanged(const std::string&value)
{

    UpdateText();
}

void Lv2cStatusTextElement::OnDropdownItemsChanged(const items_t&value)
{
    UpdateText();

}

void Lv2cStatusTextElement::UpdateText()
{
    auto selectedId = SelectedId();
    bool hasSelection = false;
    for (auto& dropdownItem: this->DropdownItems())
    {
        if (selectedId == dropdownItem.ItemId())
        {
            Text(dropdownItem.Text());
            hasSelection = true;
        }
    }
    if (!hasSelection)
    {
        Text(this->UnselectedText());
    }
    if (selectionValid != hasSelection)
    {
        this->selectionValid = hasSelection;
        typography->Style().FontStyle(hasSelection? Lv2cFontStyle::Normal: Lv2cFontStyle::Italic);
        Invalidate();
    }
}


