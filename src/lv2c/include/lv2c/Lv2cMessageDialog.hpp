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

#pragma once

#include "Lv2cDialog.hpp"
#include <string>

namespace lvtk
{
    class Lv2cButtonElement;

    enum class Lv2cMessageBoxResult
    {
        Closed,
        PrimaryButton,
        SecondaryButton
    };

    class Lv2cMessageDialog : public Lv2cDialog
    {
    public:
        using self = Lv2cMessageDialog;
        using super = Lv2cDialog;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        static ptr Create(
            Lv2cMessageDialogType messageDialogType,
            const std::string &title,
            const std::string &text)
        {
            return std::make_shared<self>(messageDialogType, title, text);
        }
        Lv2cMessageDialog();
        Lv2cMessageDialog(Lv2cMessageDialogType messageDialogType, const std::string &title, const std::string &message);

        Lv2cMessageDialogType MessageDialogType() const;
        Lv2cMessageDialog &MessageDialogType(Lv2cMessageDialogType messageDialogType);

        const std::string &Text();
        Lv2cMessageDialog &Text(const std::string &value);

        const std::string &PrimaryButtonText();
        Lv2cMessageDialog &PrimaryButtonText(const std::string &value);

        const std::string &SecondaryButtonText();
        Lv2cMessageDialog &SecondaryButtonText(const std::string &value);


        Lv2cStyle& PrimaryButtonStyle();
        Lv2cStyle& SecondaryButtonStyle();
        Lv2cStyle& IconStyle();


        const std::optional<Lv2cColor> &PrimaryButtonColor() const;
        Lv2cMessageDialog &PrimaryButtonColor(const std::optional<Lv2cColor> &value);

        const std::optional<Lv2cColor> &SecondaryButtonColor() const;
        Lv2cMessageDialog &SecondaryButtonColor(const std::optional<Lv2cColor> &value);

        const std::optional<Lv2cColor> &IconTintColor() const;
        Lv2cMessageDialog &IconTintColor(const std::optional<Lv2cColor> &value);

        const std::optional<std::string> &IconSource() const;
        Lv2cMessageDialog &IconSource(const std::optional<std::string> &value);

        Lv2cEvent<Lv2cMessageBoxResult> Result;

    protected:
        virtual void OnClosing() override;
        virtual void OnMount() override;
        virtual Lv2cElement::ptr Render() override;
        virtual void OnPrimaryButton();
        virtual void OnSecondaryButton();

    private:
        Lv2cStyle  primaryButtonStyle,secondaryButtonStyle,iconStyle;

        std::optional<Lv2cColor> primaryButtonColor;
        std::optional<Lv2cColor> secondaryButtonColor;
        std::optional<Lv2cColor> iconTintColor;
        std::optional<std::string> iconSource;
        bool resultSet = false;
        EventHandle primaryEventHandle, secondaryEventHandle;
        std::shared_ptr<Lv2cButtonElement> primaryButton;
        std::shared_ptr<Lv2cButtonElement> secondaryButton;

        Lv2cMessageDialogType messageDialogType;
        std::string text;
        std::string primaryButtonText = "OK";
        std::string secondaryButtonText;
    };
}