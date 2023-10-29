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

#include "Lv2cButtonBaseElement.hpp"
#include "Lv2cBindingProperty.hpp"
#include <memory>
#include <string>
#include "Lv2cDropdownItemElement.hpp"

typedef struct _PangoLayout PangoLayout;


namespace lvtk
{
    namespace implementation {
            class AnimatedDropdownElement;
    }
    class Lv2cTypographyElement;
    class Lv2cSvgElement;
    using selection_id_t = int64_t;
    constexpr selection_id_t INVALID_SELECTION_ID = (selection_id_t)-1;
    class Lv2cDropdownItem
    {
    public:
        Lv2cDropdownItem();
        Lv2cDropdownItem(selection_id_t itemId, const std::string &text);
        Lv2cDropdownItem(selection_id_t itemId, const std::string &text, const std::string &svgIcon);
        
        static Lv2cDropdownItem Separator();

        bool operator==(const Lv2cDropdownItem &other) const;

        selection_id_t ItemId() const;
        Lv2cDropdownItem &ItemId(selection_id_t value);

        const std::string &SvgIcon() const;
        Lv2cDropdownItem &SvgIcon(const std::string &value);

        const std::string &Text() const;
        Lv2cDropdownItem &Text(const std::string &value);

        Lv2cDropdownItem &IsSeparator(bool value);
        bool IsSeparator() const;
        

    private:
        selection_id_t itemId;
        std::string svgIcon;
        std::string text;
        bool separator;

    };

    class Lv2cDropdownElement : public Lv2cButtonBaseElement
    {
    public:
        using super = Lv2cButtonBaseElement;
        using self = Lv2cDropdownElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        Lv2cDropdownElement();
        virtual ~Lv2cDropdownElement() noexcept;

        using items_t = std::vector<Lv2cDropdownItem>;

        Lv2cBindingProperty<items_t> DropdownItemsProperty;
        Lv2cDropdownElement &DropdownItems(const items_t &items)
        {
            DropdownItemsProperty.set(items);
            return *this;
        }
        const items_t &DropdownItems() const { return DropdownItemsProperty.get(); }

        Lv2cBindingProperty<std::string> TextProperty;
        const std::string &Text() const;
        Lv2cDropdownElement &Text(const std::string &text);

        Lv2cBindingProperty<selection_id_t> SelectedIdProperty;
        selection_id_t SelectedId() const;
        Lv2cDropdownElement &SelectedId(selection_id_t text);


        Lv2cBindingProperty<std::string> UnselectedTextProperty;
        const std::string &UnselectedText() const;
        Lv2cDropdownElement &UnselectedText(const std::string &text);


        virtual void CloseDropdown();
        virtual void OpenDropdown();
        virtual bool DropdownOpen() const;

        int64_t SelectedIndex(selection_id_t selectionId) const;


    protected:

        virtual bool WantsFocus() const override;
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs&event) override;
        virtual void OnUnselectedTextChanged(const std::string&value);
        virtual void OnSelectedIdChanged(selection_id_t value);
        virtual void OnDropdownItemsChanged(const items_t&value);

        virtual bool OnClick(Lv2cMouseEventArgs &event) override;

        virtual bool OnFocus(const Lv2cFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const Lv2cFocusEventArgs &eventArgs) override;

        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnHoverStateChanged(Lv2cHoverState hoverState) override;

        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;

        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context) override
        {
            super::Measure(constraint, maxAvailable, context);
        }
        std::shared_ptr<implementation::AnimatedDropdownElement> RenderDropdown();

    protected:
        virtual void SelectPrevious();
        virtual void SelectNext();
    private:
        PangoLayout*pangoLayout = nullptr;

        void ReleaseDropdownElements();
        void UpdateText();
        void UpdateColors();
        void FireItemClick(selection_id_t itemId);

        std::shared_ptr<implementation::AnimatedDropdownElement> dropdownElement;
        Lv2cHoverColors hoverTextColors;

        bool selectionValid = false;
        std::shared_ptr<Lv2cTypographyElement> typography;
        std::shared_ptr<Lv2cSvgElement> icon;
        std::vector<EventHandle> dropdownItemEventHandles;
    };
}