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

#include "lv2c/Lv2cMessageDialog.hpp"

#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"

using namespace lv2c;

Lv2cMessageDialog::Lv2cMessageDialog()
{
    this->DefaultSize(Lv2cSize(320, 0))
        .Positioning(Lv2cWindowPositioning::CenterOnParent);

    IconStyle()
        .Width(24)
        .Height(24)
        .MarginEnd({16});
    SecondaryButtonStyle()
        .Width(80)
        .Margin({0,0,16,0})
        ;
    PrimaryButtonStyle()
        .Width(80)
        ;

}
Lv2cMessageDialog::Lv2cMessageDialog(Lv2cMessageDialogType messageDialogType, const std::string &title, const std::string &message)
    : Lv2cMessageDialog()
{

    this->messageDialogType = messageDialogType;
    this->text = message;
    switch (messageDialogType)
    {
    case Lv2cMessageDialogType::Error:
        X11WindowName("Error");
        break;
    case Lv2cMessageDialogType::Warning:
        X11WindowName("Warning");
        break;
    case Lv2cMessageDialogType::Info:
        X11WindowName("Info");
        break;
    case Lv2cMessageDialogType::Question:
        X11WindowName(title);
        break;
    }

    Title(title);
}

Lv2cMessageDialogType Lv2cMessageDialog::MessageDialogType() const
{
    return this->messageDialogType;
}
Lv2cMessageDialog &Lv2cMessageDialog::MessageDialogType(Lv2cMessageDialogType messageDialogType)
{
    this->messageDialogType = messageDialogType;
    return *this;
}

const std::string &Lv2cMessageDialog::Text()
{
    return this->text;
}
Lv2cMessageDialog &Lv2cMessageDialog::Text(const std::string &value)
{
    this->text = value;
    return *this;
}

void Lv2cMessageDialog::OnMount()
{
    primaryButton->Focus();
}
void Lv2cMessageDialog::OnClosing()
{
    if (!resultSet)
    {
        Result.Fire(Lv2cMessageBoxResult::PrimaryButton);
        resultSet = true;
    }
    primaryButton->Clicked.RemoveListener(primaryEventHandle);
    primaryEventHandle = EventHandle::InvalidHandle;
    if (secondaryButton)
    {
        secondaryButton->Clicked.RemoveListener(secondaryEventHandle);
        secondaryEventHandle = EventHandle::InvalidHandle;
    }
    super::OnClosing();
}
void Lv2cMessageDialog::OnPrimaryButton()
{
    Result.Fire(Lv2cMessageBoxResult::PrimaryButton);
    this->resultSet = true;
    Close();
}

void Lv2cMessageDialog::OnSecondaryButton()
{
    Result.Fire(Lv2cMessageBoxResult::SecondaryButton);
    this->resultSet = true;

    Close();
    Close();
}


#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC optimize("no-var-tracking") // to speed up compilation
#endif

Lv2cElement::ptr Lv2cMessageDialog::Render()
{
    auto container = Lv2cVerticalStackElement::Create();
    container->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Start)
        .Padding({24, 16, 24, 16})
        .Background(Theme().dialogBackgroundColor);
    {
        auto body = Lv2cFlexGridElement::Create();
        body->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Start)
            .FlexAlignItems(Lv2cAlignment::Start)
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            ;
        {
            auto icon = Lv2cSvgElement::Create();
            icon->Style((const Lv2cStyle&)IconStyle()); // copy not move.

            if (iconSource.has_value())
            {
                icon->Source(iconSource.value());
                if (iconTintColor.has_value())
                {
                    icon->Style().TintColor(iconTintColor.value());
                }
            } else {
                switch (MessageDialogType())
                {
                case Lv2cMessageDialogType::Info:
                    icon->Source("info.svg");
                    icon->Style().TintColor(Theme().secondaryTextColor);
                    break;
                case Lv2cMessageDialogType::Warning:
                    icon->Source("warning.svg");
                    icon->Style().TintColor(Theme().secondaryTextColor);
                    break;

                case Lv2cMessageDialogType::Error:
                    icon->Source("error.svg");
                    icon->Style().TintColor(Theme().errorColor);
                    break;
                case Lv2cMessageDialogType::Question:
                    icon->Source("help.svg"); // "(?)"
                    icon->Style().TintColor(Theme().errorColor);
                    break;
                }

                if (iconTintColor.has_value())
                {
                    icon->Style().TintColor(iconTintColor.value());
                }
            }
            body->AddChild(icon);
        }
        {
            auto typography = Lv2cTypographyElement::Create();
            typography->Text(Text()).Variant(Lv2cTypographyVariant::BodySecondary);
            typography->Style()
                .HorizontalAlignment(Lv2cAlignment::Stretch)
                .VerticalAlignment(Lv2cAlignment::Start)
                .MarginBottom({24})
                .SingleLine(false);
            body->AddChild(typography);
        }
        container->AddChild(body);
    }
    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        flexGrid->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Start)
            .FlexJustification(Lv2cFlexJustification::End)
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .FlexAlignItems(Lv2cAlignment::Center);
        if (SecondaryButtonText().length() != 0)
        {
            Lv2cButtonElement::ptr secondaryButton = Lv2cButtonElement::Create();
            this->secondaryButton = secondaryButton;
            secondaryButton->Style(secondaryButtonStyle); // copy not move.
            
            secondaryButton->Variant(Lv2cButtonVariant::BorderButton);
            secondaryButton->Text(SecondaryButtonText());

            if (secondaryButtonColor.has_value())
            {
                double alpha = Theme().buttonBorderDefaultStyle->BorderColor().get_color().A();
                secondaryButton->Style()
                    .Color(secondaryButtonColor.value())
                    .BorderColor(Lv2cColor(secondaryButtonColor.value(),alpha))
                    ;
            }
            flexGrid->AddChild(secondaryButton);

            secondaryEventHandle = secondaryButton->Clicked.AddListener([this](const Lv2cMouseEventArgs &args)
                                                    {
                OnSecondaryButton();
                return true; });
        }
        {
            Lv2cButtonElement::ptr primaryButton = Lv2cButtonElement::Create();
            this->primaryButton = primaryButton;
            primaryButton->Style(primaryButtonStyle); // copy not move.
            primaryButton->Variant(Lv2cButtonVariant::BorderButtonDefault);
            primaryButton->Text(PrimaryButtonText());
            flexGrid->AddChild(primaryButton);
            if (primaryButtonColor.has_value())
            {
                double alpha = Theme().buttonBorderDefaultStyle->BorderColor().get_color().A();
                primaryButton->Style()
                    .Color(primaryButtonColor.value())
                    .BorderColor(Lv2cColor(primaryButtonColor.value(),alpha))
                    ;
            }

            primaryEventHandle = primaryButton->Clicked.AddListener([this](const Lv2cMouseEventArgs &args)
                                                {
                OnPrimaryButton();
                return true; });
        }
        container->AddChild(flexGrid);
    }
    return container;
}
#pragma GCC diagnostic pop

const std::string &Lv2cMessageDialog::PrimaryButtonText()
{
    return primaryButtonText;
}
Lv2cMessageDialog &Lv2cMessageDialog::PrimaryButtonText(const std::string &value)
{
    primaryButtonText = value;
    return *this;
}

const std::string &Lv2cMessageDialog::SecondaryButtonText()
{
    return secondaryButtonText;
}
Lv2cMessageDialog &Lv2cMessageDialog::SecondaryButtonText(const std::string &value)
{
    secondaryButtonText = value;
    return *this;
}

const std::optional<Lv2cColor> &Lv2cMessageDialog::PrimaryButtonColor() const
{
    return this->primaryButtonColor;
}
Lv2cMessageDialog &Lv2cMessageDialog::PrimaryButtonColor(const std::optional<Lv2cColor> &value)
{
    this->primaryButtonColor = value;
    return *this;
}
const std::optional<Lv2cColor> &Lv2cMessageDialog::SecondaryButtonColor() const
{
    return this->secondaryButtonColor;
}
Lv2cMessageDialog &Lv2cMessageDialog::SecondaryButtonColor(const std::optional<Lv2cColor> &value)
{
    this->secondaryButtonColor = value;
    return *this;
}
const std::optional<Lv2cColor> &Lv2cMessageDialog::IconTintColor() const
{
    return this->iconTintColor;
}
Lv2cMessageDialog &Lv2cMessageDialog::IconTintColor(const std::optional<Lv2cColor> &value)
{
    this->iconTintColor = value;
    return *this;
}

const std::optional<std::string>& Lv2cMessageDialog::IconSource() const
{
    return iconSource;
}
Lv2cMessageDialog &Lv2cMessageDialog::IconSource(const std::optional<std::string> &value)
{
    this->iconSource = value;
    return *this;
}

Lv2cStyle& Lv2cMessageDialog::PrimaryButtonStyle()
{
    return primaryButtonStyle;
}
Lv2cStyle& Lv2cMessageDialog::SecondaryButtonStyle()
{
    return secondaryButtonStyle;
}
Lv2cStyle& Lv2cMessageDialog::IconStyle()
{
    return iconStyle;
    
}
