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

#include "LvtkContainerElement.hpp"
#include <functional>

namespace lvtk
{
    class LvtkRootElement : public LvtkContainerElement 
    {
        friend class LvtkWindow;
    public:
        using super= LvtkContainerElement;
        using ptr = std::shared_ptr<LvtkRootElement>;
        static ptr Create() { return std::make_shared<LvtkRootElement>(); }

        LvtkRootElement();

        enum class ChildType {
            Normal,
            Dialog,
            ModalDialog,
            Popup
        };
        virtual void AddPopup(std::shared_ptr<LvtkElement> child, LvtkElement*anchor, std::function<void(void)> &&onClose);

        virtual void AddChild(std::shared_ptr<LvtkElement> child) override;
        /// @brief Remove an element from the list of child elements.
        /// @param element the element to remove.
        /// @returns false if the element was not currently a child.
        virtual bool RemoveChild(std::shared_ptr<LvtkElement> element) override;
        /// @brief Removed the child at the specified position;
        /// @param index the position of the child to remove.
        /// @throws std::range_error if the index is invalid.
        virtual void RemoveChild(size_t index) override;

    protected:
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;

        virtual void UpdateMouseOver(LvtkPoint mousePosition) override;
        virtual bool FireMouseDown(LvtkMouseEventArgs&event) override;
        virtual bool FireKeyDown(const LvtkKeyboardEventArgs&event) override;
        virtual bool HandlePopupKeys(LvtkElement::ptr child, const LvtkKeyboardEventArgs&event);
        virtual void DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds) override;
    private:
        LvtkRectangle GetChildBorder(LvtkElement*element);
        LvtkRectangle GetAnchorRect(LvtkSize available, LvtkRectangle rc,LvtkElement *anchor);


        void AddChildEx(
            ChildType childType,
            LvtkElement::ptr child, 
            double zOrder, 
            LvtkElement* anchor,
            std::function<void(void)>&&onClose);

        struct ChildInfo {
            ChildType childType;
            LvtkElement::ptr child;
            double zOrder;
            LvtkElement *anchor;
            std::function<void(void)> onClose;
        };

        std::vector<ChildInfo> childInfos;
    };
}