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

#include "lvtk/LvtkStatusTextElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkPangoContext.hpp"
#include "lvtk/LvtkDropdownItemElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "pango/pangocairo.h"
#include "lvtk/LvtkPangoContext.hpp"

#include <chrono>

using namespace lvtk;


LvtkSize LvtkStatusTextElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context)
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
LvtkStatusTextElement::~LvtkStatusTextElement() noexcept
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

LvtkStatusTextElement::LvtkStatusTextElement()
{
    SelectedId(INVALID_SELECTION_ID);
    typography = LvtkTypographyElement::Create();
    typography->Variant(LvtkTypographyVariant::BodySecondary);
    this->AddChild(typography);

    typography->Style().HorizontalAlignment(LvtkAlignment::Stretch)
        .SingleLine(true)
        .Ellipsize(LvtkEllipsizeMode::End);

    TextProperty.Bind(typography->TextProperty);

    UnselectedTextProperty.SetElement(this,&LvtkStatusTextElement::OnUnselectedTextChanged);
    SelectedIdProperty.SetElement(this,&LvtkStatusTextElement::OnSelectedIdChanged);
    DropdownItemsProperty.SetElement(this,&LvtkStatusTextElement::OnDropdownItemsChanged);

}

const std::string &LvtkStatusTextElement::Text() const
{

    return TextProperty.get();
}
LvtkStatusTextElement &LvtkStatusTextElement::Text(const std::string &text)
{
    TextProperty.set(text);
    return *this;
}

void LvtkStatusTextElement::OnMount()
{
    auto &theme = Theme();
    this->AddClass(theme.statusTextStyle);
    UpdateText();
}





selection_id_t LvtkStatusTextElement::SelectedId() const
{
    return SelectedIdProperty.get();
}
LvtkStatusTextElement &LvtkStatusTextElement::SelectedId(selection_id_t selectedId)
{
    SelectedIdProperty.set(selectedId);
    return *this;
}


const std::string &LvtkStatusTextElement::UnselectedText() const
{
    return UnselectedTextProperty.get();
}
LvtkStatusTextElement &LvtkStatusTextElement::UnselectedText(const std::string &text)
{
    UnselectedTextProperty.set(text);
    return *this;
}


void LvtkStatusTextElement::OnSelectedIdChanged(selection_id_t value)
{
    UpdateText();
}
void LvtkStatusTextElement::OnUnselectedTextChanged(const std::string&value)
{

    UpdateText();
}

void LvtkStatusTextElement::OnDropdownItemsChanged(const items_t&value)
{
    UpdateText();

}

void LvtkStatusTextElement::UpdateText()
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
        typography->Style().FontStyle(hasSelection? LvtkFontStyle::Normal: LvtkFontStyle::Italic);
        Invalidate();
    }
}


