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

#include "lv2c/Lv2cDropdownItemElement.hpp"

using namespace lv2c;

#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"

Lv2cDropdownItemElement::Lv2cDropdownItemElement(selection_id_t selectionId, const std::string &text, const std::string &svgIcon, bool hasIcon)
{
    this->selectionId = selectionId;
    this->text = text;
    this->svgIcon = svgIcon;
    this->hasIcon = false;

    auto grid = Lv2cFlexGridElement::Create();
    this->AddChild(grid);
    grid->Style().FlexAlignItems(Lv2cAlignment::Center);

    if (hasIcon)
    {
        auto icon = Lv2cSvgElement::Create();
        icon->Style().Margin({0, 0, 8, 0});
        grid->AddChild(icon);
        this->icon = icon;

        icon->Source(svgIcon);
        if (svgIcon.length() == 0)
        {
            icon->Style().Visibility(Lv2cVisibility::Hidden);
        }
    }
    auto typography = Lv2cTypographyElement::Create();
    grid->AddChild(typography);
    this->typography = typography;
    typography->Text(text).Variant(Lv2cTypographyVariant::BodyPrimary);
}

Lv2cDropdownItemElement::selection_id_t Lv2cDropdownItemElement::SelectionId() const
{
    return selectionId;
}
void Lv2cDropdownItemElement::OnMount()
{
    super::OnMount();
    this->hoverTextColors = Theme().hoverTextColors;
    this->AddClass(Theme().dropdownItemStyle);
    OnHoverStateChanged(HoverState());
}
void Lv2cDropdownItemElement::OnHoverStateChanged(Lv2cHoverState hoverState)
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

const Lv2cHoverColors &Lv2cDropdownItemElement::HoverBackgroundColors()
{
    return Theme().dropdownItemHoverBackgroundColors;
}

bool Lv2cDropdownItemElement::ShowPressedState() const
{
    return true;
}
