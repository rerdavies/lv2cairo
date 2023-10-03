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

#include "lvtk/LvtkDropdownItemElement.hpp"

using namespace lvtk;

#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"

LvtkDropdownItemElement::LvtkDropdownItemElement(selection_id_t selectionId, const std::string &text, const std::string &svgIcon, bool hasIcon)
{
    this->selectionId = selectionId;
    this->text = text;
    this->svgIcon = svgIcon;
    this->hasIcon = false;

    auto grid = LvtkFlexGridElement::Create();
    this->AddChild(grid);
    grid->Style().FlexAlignItems(LvtkAlignment::Center);

    if (hasIcon)
    {
        auto icon = LvtkSvgElement::Create();
        icon->Style().Margin({0, 0, 8, 0});
        grid->AddChild(icon);
        this->icon = icon;

        icon->Source(svgIcon);
        if (svgIcon.length() == 0)
        {
            icon->Style().Visibility(LvtkVisibility::Hidden);
        }
    }
    auto typography = LvtkTypographyElement::Create();
    grid->AddChild(typography);
    this->typography = typography;
    typography->Text(text).Variant(LvtkTypographyVariant::BodyPrimary);
}

LvtkDropdownItemElement::selection_id_t LvtkDropdownItemElement::SelectionId() const
{
    return selectionId;
}
void LvtkDropdownItemElement::OnMount()
{
    super::OnMount();
    this->hoverTextColors = Theme().hoverTextColors;
    this->AddClass(Theme().dropdownItemStyle);
    OnHoverStateChanged(HoverState());
}
void LvtkDropdownItemElement::OnHoverStateChanged(LvtkHoverState hoverState)
{

    super::OnHoverStateChanged(hoverState);
    auto color = hoverTextColors.GetColor(HoverState());
    if (icon)
    {
        icon->Style().TintColor(color);
    }
    typography->Style().Color(color);
    Invalidate();
}

const LvtkHoverColors &LvtkDropdownItemElement::HoverBackgroundColors()
{
    return Theme().dropdownItemHoverBackgroundColors;
}

bool LvtkDropdownItemElement::ShowPressedState() const
{
    return true;
}
