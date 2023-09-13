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

#include "LvtkButtonBaseElement.hpp"
#include "LvtkBindingProperty.hpp"
#include <memory>
#include <string>
#include "LvtkDropdownItemElement.hpp"

typedef struct _PangoLayout PangoLayout;


namespace lvtk
{

    class LvtkTypographyElement;
    class LvtkSvgElement;
    class AnimatedDropdownElement;
    using selection_id_t = int64_t;
    constexpr selection_id_t INVALID_SELECTION_ID = (selection_id_t)-1;
    class LvtkDropdownItem
    {
    public:
        LvtkDropdownItem();
        LvtkDropdownItem(selection_id_t itemId, const std::string &text);
        LvtkDropdownItem(selection_id_t itemId, const std::string &text, const std::string &svgIcon);
        
        static LvtkDropdownItem Separator();

        bool operator==(const LvtkDropdownItem &other) const;

        selection_id_t ItemId() const;
        LvtkDropdownItem &ItemId(selection_id_t value);

        const std::string &SvgIcon() const;
        LvtkDropdownItem &SvgIcon(const std::string &value);

        const std::string &Text() const;
        LvtkDropdownItem &Text(const std::string &value);

        LvtkDropdownItem &IsSeparator(bool value);
        bool IsSeparator() const;
        

    private:
        selection_id_t itemId;
        std::string svgIcon;
        std::string text;
        bool separator;

    };

    class LvtkDropdownElement : public LvtkButtonBaseElement
    {
    public:
        using super = LvtkButtonBaseElement;
        using self = LvtkDropdownElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        LvtkDropdownElement();
        virtual ~LvtkDropdownElement() noexcept;

        using items_t = std::vector<LvtkDropdownItem>;

        LvtkBindingProperty<items_t> DropdownItemsProperty;
        LvtkDropdownElement &DropdownItems(const items_t &items)
        {
            DropdownItemsProperty.set(items);
            return *this;
        }
        const items_t &DropdownItems() const { return DropdownItemsProperty.get(); }

        LvtkBindingProperty<std::string> TextProperty;
        const std::string &Text() const;
        LvtkDropdownElement &Text(const std::string &text);

        LvtkBindingProperty<selection_id_t> SelectedIdProperty;
        selection_id_t SelectedId() const;
        LvtkDropdownElement &SelectedId(selection_id_t text);


        LvtkBindingProperty<std::string> UnselectedTextProperty;
        const std::string &UnselectedText() const;
        LvtkDropdownElement &UnselectedText(const std::string &text);


        virtual void CloseDropdown();
        virtual void OpenDropdown();
        virtual bool DropdownOpen() const;

        int64_t SelectedIndex(selection_id_t selectionId) const;


    protected:

        virtual bool WantsFocus() const override;
        virtual bool OnKeyDown(const LvtkKeyboardEventArgs&event) override;
        virtual void OnUnselectedTextChanged(const std::string&value);
        virtual void OnSelectedIdChanged(selection_id_t value);
        virtual void OnDropdownItemsChanged(const items_t&value);

        virtual bool OnClick(LvtkMouseEventArgs &event) override;

        virtual bool OnFocus(const LvtkFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const LvtkFocusEventArgs &eventArgs) override;

        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnHoverStateChanged(LvtkHoverState hoverState) override;

        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;

        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable, LvtkDrawingContext &context) override
        {
            super::Measure(constraint, maxAvailable, context);
        }
        std::shared_ptr<AnimatedDropdownElement> RenderDropdown();

    protected:
        virtual void SelectPrevious();
        virtual void SelectNext();
    private:
        PangoLayout*pangoLayout = nullptr;

        void ReleaseDropdownElements();
        void UpdateText();
        void UpdateColors();
        void FireItemClick(selection_id_t itemId);

        std::shared_ptr<AnimatedDropdownElement> dropdownElement;
        LvtkHoverColors hoverTextColors;

        bool selectionValid = false;
        std::shared_ptr<LvtkTypographyElement> typography;
        std::shared_ptr<LvtkSvgElement> icon;
        std::vector<EventHandle> dropdownItemEventHandles;
    };
}