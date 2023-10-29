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

#include "lv2c_ui/Lv2FileElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include <filesystem>

#define XK_MISCELLANY
#define XK_LATIN1
#include "X11/keysymdef.h"


using namespace lvtk;
using namespace lvtk::ui;

Lv2FileElement::Lv2FileElement()
{
    auto flexGrid = Lv2cFlexGridElement::Create();
    this->AddChild(flexGrid);
    flexGrid->Style()
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .FlexAlignItems(Lv2cAlignment::Center);

    typography = Lv2cTypographyElement::Create();
    flexGrid->AddChild(typography);
    typography->Style().HorizontalAlignment(Lv2cAlignment::Stretch).Ellipsize(Lv2cEllipsizeMode::End).SingleLine(true);

    auto icon = Lv2cSvgElement::Create();
    this->icon = icon;
    icon->Source("ic_more.svg");
    icon->Style()
        .Width(18)
        .Height(18)
        .Margin({4, 0, 0, 0});

    flexGrid->AddChild(icon);

    DropdownTextProperty.Bind(typography->TextProperty);

    FilenameProperty.SetElement(this,&Lv2FileElement::OnFilenameChanged);

}

void Lv2FileElement::OnFilenameChanged(const std::string&filename)
{
    if (filename.length() == 0)
    {
        DropdownText("");
    } else {
        std::filesystem::path path { filename};
        std::string fileOnly = path.stem();
        DropdownText(fileOnly);
    }

}


bool Lv2FileElement::WantsFocus() const 
{
    return true;
}

void Lv2FileElement::OnMount()
{
    auto &theme = Theme();

    this->ClearClasses();

    super::OnMount();
    this->AddClass(theme.dropdownUnderlineStyle);
    this->hoverTextColors = theme.hoverTextColors;

    UpdateColors();
}

void Lv2FileElement::UpdateColors()
{
    Lv2cColor color = hoverTextColors.GetColor(HoverState());
    this->icon->Style().TintColor(color);
}


void Lv2FileElement::OnHoverStateChanged(Lv2cHoverState hoverState)
{
    UpdateColors();
    this->Invalidate();
}


bool Lv2FileElement::OnKeyDown(const Lv2cKeyboardEventArgs&event) 
{
    if (event.keysymValid)
    {
        switch (event.keysym)
        {
        case XK_space:
        case XK_KP_Space:
        case XK_Return:
        case XK_KP_Enter:
            FireKeyboardClick();    
            return true;

        default:
            break;
        }
    }
    return false;
}
