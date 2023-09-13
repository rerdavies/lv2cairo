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

#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include <string.h>

#include "lvtk/LvtkDrawingContext.hpp"

#define XK_LATIN1
#define XK_MISCELLANY
#include "X11/keysymdef.h"

using namespace lvtk;



void LvtkButtonElement::OnTextChanged(const std::string &text)
{
    if (this->typography && text.length() != 0)
    {
        // binding does the work.
    }
    else
    {
        RebuildChildren();
    }
}

void  LvtkButtonElement::OnIconChanged(const std::string&value)
{
    if (this->icon && value.length() != 0)
    {
        // binding does the work.
    } else {
        RebuildChildren();
    }
}

LvtkButtonElement::LvtkButtonElement()
{
    TextProperty.SetElement(this,&LvtkButtonElement::OnTextChanged);
    IconProperty.SetElement(this,&LvtkButtonElement::OnIconChanged);
}

LvtkButtonElement &LvtkButtonElement::Variant(LvtkButtonVariant variant)
{
    if (this->variant != variant)
    {
        this->variant = variant;
        RebuildChildren();
    }
    return *this;
}
LvtkButtonVariant LvtkButtonElement::Variant() const
{
    return variant;
}

void LvtkButtonElement::OnMount()
{
    RebuildChildren();
    super::OnMount();
}

void LvtkButtonElement::RebuildChildren()
{
    if (!IsMounted())
        return;
    children.resize(0);

    std::vector<LvtkElement::ptr> children;
    typography = nullptr;
    if (Text().length() != 0)
    {
        this->typography = LvtkTypographyElement::Create();
        TextProperty.Bind(typography->TextProperty);
        typography->Variant(LvtkTypographyVariant::Inherit);
        typography->Style().HorizontalAlignment(LvtkAlignment::Center);
        typography->Style().VerticalAlignment(LvtkAlignment::Center);
        // spacer.
        this->icon = LvtkElement::Create();
        this->icon->Style()
            .Width(0)
            .Height(20)
            ;
    }
    bool hasIcon = Icon().length();
    if (hasIcon)
    {
        auto element = LvtkSvgElement::Create();
        this->icon = element;
        this->IconProperty.Bind(element->SourceProperty);
        element->Style()
            .Width(20)
            .Height(20)
            ;
    }
    if (this->typography && hasIcon)
    {
        auto grid = LvtkFlexGridElement::Create();
        grid->Style()
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .HorizontalAlignment(LvtkAlignment::Start)
            .FlexColumnGap(8)
            ;
        typography->Style()
            .TextAlign(LvtkTextAlign::Start)
            ;
        grid->AddChild(icon);
        grid->AddChild(typography);
        children.push_back(grid);
    } else if (this->typography)
    {
        children.push_back(typography);
        children.push_back(icon);
    } else if (this->icon)
    {
        children.push_back(icon);
    }

    this->Children(children);

    const LvtkTheme &theme = Theme();
    switch (this->variant)
    {
    case LvtkButtonVariant::Dialog:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogStyle);
        break;
    case LvtkButtonVariant::DialogPrimary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogPrimaryStyle);
        break;
    case LvtkButtonVariant::DialogSecondary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogSecondaryStyle);
        break;
    case LvtkButtonVariant::BorderButton:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderStyle);
        break;
    case LvtkButtonVariant::BorderButtonDefault:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderDefaultStyle);
        break;
    case LvtkButtonVariant::BorderButtonPrimary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderPrimaryStyle);
        break;
    case LvtkButtonVariant::BorderButtonSecondary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderSecondaryStyle);
        break;
    case LvtkButtonVariant::ImageButton:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.imageButtonStyle);
        break;
    case LvtkButtonVariant::ImageButtonPrimary:
            this->hoverColors = theme.invertedHoverBackgroundColors;
        this->Classes(theme.imageButtonPrimaryStyle);
        break;
    case LvtkButtonVariant::ImageButtonSecondary:
        this->hoverColors = theme.invertedHoverBackgroundColors;
        this->Classes(theme.imageButtonSecondaryStyle);
        break;
    default:
        throw std::runtime_error("Invalid variant.");
    }
    if (this->icon)
    {
        double iconSize = Style().IconSize();
        if (iconSize != 0)
        {
            icon->Style().Width(iconSize).Height(iconSize);
        }
        switch (this->Variant())
        {
            case LvtkButtonVariant::ImageButton:
            case LvtkButtonVariant:: ImageButtonPrimary:
            case LvtkButtonVariant:: ImageButtonSecondary:
                icon->Style()
                    .HorizontalAlignment(LvtkAlignment::Center)
                    .VerticalAlignment(LvtkAlignment::Center)
                    ;
                break;
            default:
                icon->Style()
                    .HorizontalAlignment(LvtkAlignment::Start)
                    .VerticalAlignment(LvtkAlignment::Center)
                    ;
                break;

        }

    }

}

bool LvtkButtonElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (super::OnKeyDown(event))
    {
        return true;
    }
    if (event.textValid)
    {
        KeyPressed(strcmp(event.text," ") == 0);
    }
    return false;
}
bool LvtkButtonElement::OnKeycodeDown(const LvtkKeyboardEventArgs &event)
{
    return super::OnKeycodeDown(event);
}
bool LvtkButtonElement::OnKeycodeUp(const LvtkKeyboardEventArgs &event)
{

    if (super::OnKeycodeUp(event))
    {
        KeyPressed(false);
        return true;
    }

    if (KeyPressed())
    {
        LvtkMouseEventArgs args;
        args.modifierState = event.modifierState;
        args.point = LvtkPoint(this->ClientSize().Width() / 2, this->ClientSize().Height() / 2);
        this->Clicked.Fire(args);
        KeyPressed(false);
        return true;
    }
    return false;
}

bool LvtkButtonElement::KeyPressed() const
{
    return keyPressed;
}
void LvtkButtonElement::KeyPressed(bool value)
{
    if (this->keyPressed != value)
    {
        this->keyPressed = value;
        if (keyPressed)
        {
            HoverState(HoverState() + LvtkHoverState::Pressed);
        }
        else
        {
            HoverState(HoverState() - LvtkHoverState::Pressed);
        }
    }
}

bool LvtkButtonElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    KeyPressed(false);
    return super::OnMouseDown(event);
}

bool LvtkButtonElement::ShowPressedState() const
{
    return KeyPressed();
}

void LvtkButtonElement::Measure(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context)  {
    
    super::Measure(constraint,maxAvailable,context);
}


const LvtkHoverColors &LvtkButtonElement::HoverBackgroundColors() {
    return this->hoverColors;
}
