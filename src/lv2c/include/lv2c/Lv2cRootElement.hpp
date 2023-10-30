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

#include "Lv2cContainerElement.hpp"
#include <functional>

namespace lv2c
{
    class Lv2cRootElement : public Lv2cContainerElement 
    {
        friend class Lv2cWindow;
    public:
        using super= Lv2cContainerElement;
        using ptr = std::shared_ptr<Lv2cRootElement>;
        static ptr Create() { return std::make_shared<Lv2cRootElement>(); }

        Lv2cRootElement();

        enum class ChildType {
            Normal,
            Dialog,
            ModalDialog,
            Popup
        };
        virtual void AddPopup(std::shared_ptr<Lv2cElement> child, Lv2cElement*anchor, std::function<void(void)> &&onClose);

        virtual void AddChild(std::shared_ptr<Lv2cElement> child) override;
        /// @brief Remove an element from the list of child elements.
        /// @param element the element to remove.
        /// @returns false if the element was not currently a child.
        virtual bool RemoveChild(std::shared_ptr<Lv2cElement> element) override;
        /// @brief Removed the child at the specified position;
        /// @param index the position of the child to remove.
        /// @throws std::range_error if the index is invalid.
        virtual void RemoveChild(size_t index) override;

    protected:
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;

        virtual void UpdateMouseOver(Lv2cPoint mousePosition) override;
        virtual bool FireMouseDown(Lv2cMouseEventArgs&event) override;
        virtual bool FireKeyDown(const Lv2cKeyboardEventArgs&event) override;
        virtual bool HandlePopupKeys(Lv2cElement::ptr child, const Lv2cKeyboardEventArgs&event);
        virtual void DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds) override;
    private:
        Lv2cRectangle GetChildBorder(Lv2cElement*element);
        Lv2cRectangle GetAnchorRect(Lv2cSize available, Lv2cRectangle rc,Lv2cElement *anchor);


        void AddChildEx(
            ChildType childType,
            Lv2cElement::ptr child, 
            double zOrder, 
            Lv2cElement* anchor,
            std::function<void(void)>&&onClose);

        struct ChildInfo {
            ChildType childType;
            Lv2cElement::ptr child;
            double zOrder;
            Lv2cElement *anchor;
            std::function<void(void)> onClose;
        };

        std::vector<ChildInfo> childInfos;
    };
}