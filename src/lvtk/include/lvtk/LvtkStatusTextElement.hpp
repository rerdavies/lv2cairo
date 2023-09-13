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

#include "LvtkDropdownElement.hpp"



namespace lvtk
{

    /// @brief Display text from a list of items.
    /// Behaves like LvtkDropdownElement, but read-only. 
    /// 
    /// Display the text from a list of LvtkDropdownItem's based on the value 
    /// of the SelectedId property.
    class LvtkStatusTextElement : public LvtkContainerElement
    {
    public:
        using super = LvtkContainerElement;
        using self = LvtkStatusTextElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        LvtkStatusTextElement();
        virtual ~LvtkStatusTextElement() noexcept;

        using items_t = std::vector<LvtkDropdownItem>;

        LvtkBindingProperty<items_t> DropdownItemsProperty;
        LvtkStatusTextElement &DropdownItems(const items_t &items)
        {
            DropdownItemsProperty.set(items);
            return *this;
        }
        const items_t &DropdownItems() const { return DropdownItemsProperty.get(); }


        LvtkBindingProperty<selection_id_t> SelectedIdProperty;
        selection_id_t SelectedId() const;
        LvtkStatusTextElement &SelectedId(selection_id_t text);


        LvtkBindingProperty<std::string> UnselectedTextProperty;
        const std::string &UnselectedText() const;
        LvtkStatusTextElement &UnselectedText(const std::string &text);


    protected:
        virtual void OnUnselectedTextChanged(const std::string&value);
        virtual void OnSelectedIdChanged(selection_id_t value);
        virtual void OnDropdownItemsChanged(const items_t&value);

        virtual void OnMount() override;

        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;

        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable, LvtkDrawingContext &context) override
        {
            super::Measure(constraint, maxAvailable, context);
        }

    private:
        bool selectionValid = false;
        LvtkBindingProperty<std::string> TextProperty;
        const std::string &Text() const;
        LvtkStatusTextElement &Text(const std::string &text);

        std::shared_ptr<LvtkTypographyElement> typography;

        PangoLayout*pangoLayout = nullptr;

        void UpdateText();

    };
}