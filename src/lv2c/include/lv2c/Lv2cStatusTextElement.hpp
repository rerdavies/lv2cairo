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

#include "Lv2cDropdownElement.hpp"



namespace lvtk
{

    /// @brief Display text from a list of items.
    /// Behaves like Lv2cDropdownElement, but read-only. 
    /// 
    /// Display the text from a list of Lv2cDropdownItem's based on the value 
    /// of the SelectedId property.
    class Lv2cStatusTextElement : public Lv2cContainerElement
    {
    public:
        using super = Lv2cContainerElement;
        using self = Lv2cStatusTextElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        Lv2cStatusTextElement();
        virtual ~Lv2cStatusTextElement() noexcept;

        using items_t = std::vector<Lv2cDropdownItem>;

        Lv2cBindingProperty<items_t> DropdownItemsProperty;
        Lv2cStatusTextElement &DropdownItems(const items_t &items)
        {
            DropdownItemsProperty.set(items);
            return *this;
        }
        const items_t &DropdownItems() const { return DropdownItemsProperty.get(); }


        Lv2cBindingProperty<selection_id_t> SelectedIdProperty;
        selection_id_t SelectedId() const;
        Lv2cStatusTextElement &SelectedId(selection_id_t text);


        Lv2cBindingProperty<std::string> UnselectedTextProperty;
        const std::string &UnselectedText() const;
        Lv2cStatusTextElement &UnselectedText(const std::string &text);


    protected:
        virtual void OnUnselectedTextChanged(const std::string&value);
        virtual void OnSelectedIdChanged(selection_id_t value);
        virtual void OnDropdownItemsChanged(const items_t&value);

        virtual void OnMount() override;

        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;

        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context) override
        {
            super::Measure(constraint, maxAvailable, context);
        }

    private:
        bool selectionValid = false;
        Lv2cBindingProperty<std::string> TextProperty;
        const std::string &Text() const;
        Lv2cStatusTextElement &Text(const std::string &text);

        std::shared_ptr<Lv2cTypographyElement> typography;

        PangoLayout*pangoLayout = nullptr;

        void UpdateText();

    };
}