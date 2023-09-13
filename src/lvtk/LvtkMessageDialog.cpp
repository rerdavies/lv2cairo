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

#include "lvtk/LvtkMessageDialog.hpp"

#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"

using namespace lvtk;

LvtkMessageDialog::LvtkMessageDialog()
{
    this->DefaultSize(LvtkSize(320, 0))
        .Positioning(LvtkWindowPositioning::CenterOnParent);

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
LvtkMessageDialog::LvtkMessageDialog(LvtkMessageDialogType messageDialogType, const std::string &title, const std::string &message)
    : LvtkMessageDialog()
{

    this->messageDialogType = messageDialogType;
    this->text = message;
    switch (messageDialogType)
    {
    case LvtkMessageDialogType::Error:
        X11WindowName("Error");
        break;
    case LvtkMessageDialogType::Warning:
        X11WindowName("Warning");
        break;
    case LvtkMessageDialogType::Info:
        X11WindowName("Info");
        break;
    case LvtkMessageDialogType::Question:
        X11WindowName(title);
        break;
    }

    Title(title);
}

LvtkMessageDialogType LvtkMessageDialog::MessageDialogType() const
{
    return this->messageDialogType;
}
LvtkMessageDialog &LvtkMessageDialog::MessageDialogType(LvtkMessageDialogType messageDialogType)
{
    this->messageDialogType = messageDialogType;
    return *this;
}

const std::string &LvtkMessageDialog::Text()
{
    return this->text;
}
LvtkMessageDialog &LvtkMessageDialog::Text(const std::string &value)
{
    this->text = value;
    return *this;
}

void LvtkMessageDialog::OnMount()
{
    primaryButton->Focus();
}
void LvtkMessageDialog::OnClosing()
{
    if (!resultSet)
    {
        Result.Fire(LvtkMessageBoxResult::PrimaryButton);
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
void LvtkMessageDialog::OnPrimaryButton()
{
    Result.Fire(LvtkMessageBoxResult::PrimaryButton);
    this->resultSet = true;
    Close();
}

void LvtkMessageDialog::OnSecondaryButton()
{
    Result.Fire(LvtkMessageBoxResult::SecondaryButton);
    this->resultSet = true;

    Close();
    Close();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC optimize("no-var-tracking") // to speed up compilation

LvtkElement::ptr LvtkMessageDialog::Render()
{
    auto container = LvtkVerticalStackElement::Create();
    container->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Start)
        .Padding({24, 16, 24, 16})
        .Background(Theme().dialogBackgroundColor);
    {
        auto body = LvtkFlexGridElement::Create();
        body->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Start)
            .FlexAlignItems(LvtkAlignment::Start)
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            ;
        {
            auto icon = LvtkSvgElement::Create();
            icon->Style((const LvtkStyle&)IconStyle()); // copy not move.

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
                case LvtkMessageDialogType::Info:
                    icon->Source("info.svg");
                    icon->Style().TintColor(Theme().secondaryTextColor);
                    break;
                case LvtkMessageDialogType::Warning:
                    icon->Source("warning.svg");
                    icon->Style().TintColor(Theme().secondaryTextColor);
                    break;

                case LvtkMessageDialogType::Error:
                    icon->Source("error.svg");
                    icon->Style().TintColor(Theme().errorColor);
                    break;
                case LvtkMessageDialogType::Question:
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
            auto typography = LvtkTypographyElement::Create();
            typography->Text(Text()).Variant(LvtkTypographyVariant::BodySecondary);
            typography->Style()
                .HorizontalAlignment(LvtkAlignment::Stretch)
                .VerticalAlignment(LvtkAlignment::Start)
                .MarginBottom({24})
                .SingleLine(false);
            body->AddChild(typography);
        }
        container->AddChild(body);
    }
    {
        auto flexGrid = LvtkFlexGridElement::Create();
        flexGrid->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Start)
            .FlexJustification(LvtkFlexJustification::End)
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .FlexAlignItems(LvtkAlignment::Center);
        if (SecondaryButtonText().length() != 0)
        {
            LvtkButtonElement::ptr secondaryButton = LvtkButtonElement::Create();
            this->secondaryButton = secondaryButton;
            secondaryButton->Style(secondaryButtonStyle); // copy not move.
            
            secondaryButton->Variant(LvtkButtonVariant::BorderButton);
            secondaryButton->Text(SecondaryButtonText());

            if (secondaryButtonColor.has_value())
            {
                double alpha = Theme().buttonBorderDefaultStyle->BorderColor().get_color().A();
                secondaryButton->Style()
                    .Color(secondaryButtonColor.value())
                    .BorderColor(LvtkColor(secondaryButtonColor.value(),alpha))
                    ;
            }
            flexGrid->AddChild(secondaryButton);

            secondaryEventHandle = secondaryButton->Clicked.AddListener([this](const LvtkMouseEventArgs &args)
                                                    {
                OnSecondaryButton();
                return true; });
        }
        {
            LvtkButtonElement::ptr primaryButton = LvtkButtonElement::Create();
            this->primaryButton = primaryButton;
            primaryButton->Style(primaryButtonStyle); // copy not move.
            primaryButton->Variant(LvtkButtonVariant::BorderButtonDefault);
            primaryButton->Text(PrimaryButtonText());
            flexGrid->AddChild(primaryButton);
            if (primaryButtonColor.has_value())
            {
                double alpha = Theme().buttonBorderDefaultStyle->BorderColor().get_color().A();
                primaryButton->Style()
                    .Color(primaryButtonColor.value())
                    .BorderColor(LvtkColor(primaryButtonColor.value(),alpha))
                    ;
            }

            primaryEventHandle = primaryButton->Clicked.AddListener([this](const LvtkMouseEventArgs &args)
                                                {
                OnPrimaryButton();
                return true; });
        }
        container->AddChild(flexGrid);
    }
    return container;
}
#pragma GCC diagnostic pop

const std::string &LvtkMessageDialog::PrimaryButtonText()
{
    return primaryButtonText;
}
LvtkMessageDialog &LvtkMessageDialog::PrimaryButtonText(const std::string &value)
{
    primaryButtonText = value;
    return *this;
}

const std::string &LvtkMessageDialog::SecondaryButtonText()
{
    return secondaryButtonText;
}
LvtkMessageDialog &LvtkMessageDialog::SecondaryButtonText(const std::string &value)
{
    secondaryButtonText = value;
    return *this;
}

const std::optional<LvtkColor> &LvtkMessageDialog::PrimaryButtonColor() const
{
    return this->primaryButtonColor;
}
LvtkMessageDialog &LvtkMessageDialog::PrimaryButtonColor(const std::optional<LvtkColor> &value)
{
    this->primaryButtonColor = value;
    return *this;
}
const std::optional<LvtkColor> &LvtkMessageDialog::SecondaryButtonColor() const
{
    return this->secondaryButtonColor;
}
LvtkMessageDialog &LvtkMessageDialog::SecondaryButtonColor(const std::optional<LvtkColor> &value)
{
    this->secondaryButtonColor = value;
    return *this;
}
const std::optional<LvtkColor> &LvtkMessageDialog::IconTintColor() const
{
    return this->iconTintColor;
}
LvtkMessageDialog &LvtkMessageDialog::IconTintColor(const std::optional<LvtkColor> &value)
{
    this->iconTintColor = value;
    return *this;
}

const std::optional<std::string>& LvtkMessageDialog::IconSource() const
{
    return iconSource;
}
LvtkMessageDialog &LvtkMessageDialog::IconSource(const std::optional<std::string> &value)
{
    this->iconSource = value;
    return *this;
}

LvtkStyle& LvtkMessageDialog::PrimaryButtonStyle()
{
    return primaryButtonStyle;
}
LvtkStyle& LvtkMessageDialog::SecondaryButtonStyle()
{
    return secondaryButtonStyle;
}
LvtkStyle& LvtkMessageDialog::IconStyle()
{
    return iconStyle;
    
}
