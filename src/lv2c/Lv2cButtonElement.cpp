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

#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include <string.h>

#include "lv2c/Lv2cDrawingContext.hpp"

#define XK_LATIN1
#define XK_MISCELLANY
#include "X11/keysymdef.h"

using namespace lv2c;



void Lv2cButtonElement::OnTextChanged(const std::string &text)
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

void  Lv2cButtonElement::OnIconChanged(const std::string&value)
{
    if (this->icon && value.length() != 0)
    {
        // binding does the work.
    } else {
        RebuildChildren();
    }
}

Lv2cButtonElement::Lv2cButtonElement()
{
    TextProperty.SetElement(this,&Lv2cButtonElement::OnTextChanged);
    IconProperty.SetElement(this,&Lv2cButtonElement::OnIconChanged);
}

Lv2cButtonElement &Lv2cButtonElement::Variant(Lv2cButtonVariant variant)
{
    if (this->variant != variant)
    {
        this->variant = variant;
        RebuildChildren();
    }
    return *this;
}
Lv2cButtonVariant Lv2cButtonElement::Variant() const
{
    return variant;
}

void Lv2cButtonElement::OnMount()
{
    RebuildChildren();
    super::OnMount();
}

void Lv2cButtonElement::RebuildChildren()
{
    if (!IsMounted())
        return;
    children.resize(0);

    std::vector<Lv2cElement::ptr> children;
    typography = nullptr;
    if (Text().length() != 0)
    {
        this->typography = Lv2cTypographyElement::Create();
        TextProperty.Bind(typography->TextProperty);
        typography->Variant(Lv2cTypographyVariant::Inherit);
        typography->Style().HorizontalAlignment(Lv2cAlignment::Center);
        typography->Style().VerticalAlignment(Lv2cAlignment::Center);
        // spacer.
        this->icon = Lv2cElement::Create();
        this->icon->Style()
            .Width(0)
            .Height(20)
            ;
    }
    bool hasIcon = Icon().length();
    if (hasIcon)
    {
        auto element = Lv2cSvgElement::Create();
        this->icon = element;
        this->IconProperty.Bind(element->SourceProperty);
        element->Style()
            .Width(20)
            .Height(20)
            ;
    }
    if (this->typography && hasIcon)
    {
        auto grid = Lv2cFlexGridElement::Create();
        grid->Style()
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .HorizontalAlignment(Lv2cAlignment::Start)
            .ColumnGap(8)
            ;
        typography->Style()
            .TextAlign(Lv2cTextAlign::Start)
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

    const Lv2cTheme &theme = Theme();
    switch (this->variant)
    {
    case Lv2cButtonVariant::Dialog:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogStyle);
        break;
    case Lv2cButtonVariant::DialogPrimary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogPrimaryStyle);
        break;
    case Lv2cButtonVariant::DialogSecondary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonDialogSecondaryStyle);
        break;
    case Lv2cButtonVariant::BorderButton:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderStyle);
        break;
    case Lv2cButtonVariant::BorderButtonDefault:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderDefaultStyle);
        break;
    case Lv2cButtonVariant::BorderButtonPrimary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderPrimaryStyle);
        break;
    case Lv2cButtonVariant::BorderButtonSecondary:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.buttonBorderSecondaryStyle);
        break;
    case Lv2cButtonVariant::ImageButton:
        this->hoverColors = theme.hoverBackgroundColors;
        this->Classes(theme.imageButtonStyle);
        break;
    case Lv2cButtonVariant::ImageButtonPrimary:
            this->hoverColors = theme.invertedHoverBackgroundColors;
        this->Classes(theme.imageButtonPrimaryStyle);
        break;
    case Lv2cButtonVariant::ImageButtonSecondary:
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
            case Lv2cButtonVariant::ImageButton:
            case Lv2cButtonVariant:: ImageButtonPrimary:
            case Lv2cButtonVariant:: ImageButtonSecondary:
                icon->Style()
                    .HorizontalAlignment(Lv2cAlignment::Center)
                    .VerticalAlignment(Lv2cAlignment::Center)
                    ;
                break;
            default:
                icon->Style()
                    .HorizontalAlignment(Lv2cAlignment::Start)
                    .VerticalAlignment(Lv2cAlignment::Center)
                    ;
                break;

        }

    }

}

bool Lv2cButtonElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
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
bool Lv2cButtonElement::OnKeycodeDown(const Lv2cKeyboardEventArgs &event)
{
    return super::OnKeycodeDown(event);
}
bool Lv2cButtonElement::OnKeycodeUp(const Lv2cKeyboardEventArgs &event)
{

    if (super::OnKeycodeUp(event))
    {
        KeyPressed(false);
        return true;
    }

    if (KeyPressed())
    {
        Lv2cMouseEventArgs args;
        args.modifierState = event.modifierState;
        args.point = Lv2cPoint(this->ClientSize().Width() / 2, this->ClientSize().Height() / 2);
        this->Clicked.Fire(args);
        KeyPressed(false);
        return true;
    }
    return false;
}

bool Lv2cButtonElement::KeyPressed() const
{
    return keyPressed;
}
void Lv2cButtonElement::KeyPressed(bool value)
{
    if (this->keyPressed != value)
    {
        this->keyPressed = value;
        if (keyPressed)
        {
            HoverState(HoverState() + Lv2cHoverState::Pressed);
        }
        else
        {
            HoverState(HoverState() - Lv2cHoverState::Pressed);
        }
    }
}

bool Lv2cButtonElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    KeyPressed(false);
    return super::OnMouseDown(event);
}

bool Lv2cButtonElement::ShowPressedState() const
{
    return KeyPressed();
}

void Lv2cButtonElement::Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context)  {
    
    super::Measure(constraint,maxAvailable,context);
}


const Lv2cHoverColors &Lv2cButtonElement::HoverBackgroundColors() {
    return this->hoverColors;
}
