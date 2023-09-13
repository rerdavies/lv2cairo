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

#pragma once

#include "LvtkDialog.hpp"
#include <string>

namespace lvtk
{
    class LvtkButtonElement;

    enum class LvtkMessageBoxResult
    {
        Closed,
        PrimaryButton,
        SecondaryButton
    };

    class LvtkMessageDialog : public LvtkDialog
    {
    public:
        using self = LvtkMessageDialog;
        using super = LvtkDialog;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        static ptr Create(
            LvtkMessageDialogType messageDialogType,
            const std::string &title,
            const std::string &text)
        {
            return std::make_shared<self>(messageDialogType, title, text);
        }
        LvtkMessageDialog();
        LvtkMessageDialog(LvtkMessageDialogType messageDialogType, const std::string &title, const std::string &message);

        LvtkMessageDialogType MessageDialogType() const;
        LvtkMessageDialog &MessageDialogType(LvtkMessageDialogType messageDialogType);

        const std::string &Text();
        LvtkMessageDialog &Text(const std::string &value);

        const std::string &PrimaryButtonText();
        LvtkMessageDialog &PrimaryButtonText(const std::string &value);

        const std::string &SecondaryButtonText();
        LvtkMessageDialog &SecondaryButtonText(const std::string &value);


        LvtkStyle& PrimaryButtonStyle();
        LvtkStyle& SecondaryButtonStyle();
        LvtkStyle& IconStyle();


        const std::optional<LvtkColor> &PrimaryButtonColor() const;
        LvtkMessageDialog &PrimaryButtonColor(const std::optional<LvtkColor> &value);

        const std::optional<LvtkColor> &SecondaryButtonColor() const;
        LvtkMessageDialog &SecondaryButtonColor(const std::optional<LvtkColor> &value);

        const std::optional<LvtkColor> &IconTintColor() const;
        LvtkMessageDialog &IconTintColor(const std::optional<LvtkColor> &value);

        const std::optional<std::string> &IconSource() const;
        LvtkMessageDialog &IconSource(const std::optional<std::string> &value);

        LvtkEvent<LvtkMessageBoxResult> Result;

    protected:
        virtual void OnClosing() override;
        virtual void OnMount() override;
        virtual LvtkElement::ptr Render() override;
        virtual void OnPrimaryButton();
        virtual void OnSecondaryButton();

    private:
        LvtkStyle  primaryButtonStyle,secondaryButtonStyle,iconStyle;

        std::optional<LvtkColor> primaryButtonColor;
        std::optional<LvtkColor> secondaryButtonColor;
        std::optional<LvtkColor> iconTintColor;
        std::optional<std::string> iconSource;
        bool resultSet = false;
        EventHandle primaryEventHandle, secondaryEventHandle;
        std::shared_ptr<LvtkButtonElement> primaryButton;
        std::shared_ptr<LvtkButtonElement> secondaryButton;

        LvtkMessageDialogType messageDialogType;
        std::string text;
        std::string primaryButtonText = "OK";
        std::string secondaryButtonText;
    };
}