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

#include "LvtkElement.hpp"

#pragma once

namespace lvtk {
    class LvtkContainerElement: public LvtkElement {
    public:
        virtual const char* Tag() const override { return "Container";}

        using super = LvtkElement;
        using self = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        virtual ~LvtkContainerElement() { }



        /// @brief Add an element to the list of child elements.
        /// @param child The element to add.
        /// @param position Where to add the element.
        virtual void AddChild(std::shared_ptr<LvtkElement> child);
        virtual void AddChild(std::shared_ptr<LvtkElement> child,size_t position);


        /// @brief Remove an element from the list of child elements.
        /// @param element the element to remove.
        /// @returns false if the element was not currently a child.
        virtual bool RemoveChild(std::shared_ptr<LvtkElement> element);
        /// @brief Removed the child at the specified position;
        /// @param index the position of the child to remove.
        /// @throws std::range_error if the index is invalid.
        virtual void RemoveChild(size_t index);

        /// @brief Get the child at the specified position;
        /// @param index the position of the child to return.
        /// @returns the specified child.
        /// @throws std::range_error if the index is invalid.
        //virtual LvtkElement::ptr Child(size_t index);


        /// @brief Get the specified logical child element.
        /// @param index the index of the child to return.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual const LvtkElement *Child(size_t index) const;
        virtual LvtkElement::ptr Child(size_t index);

        /// @brief Get the number of direct child elements.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual size_t ChildCount() const;


        /// @brief Replace all children with the supplied list of new children.
        /// @param children the list of children with which to replace the current children.
        virtual void Children(const std::vector<LvtkElement::ptr>&children);

        /// @brief Get a list of the current logical children. 
        /// Caution: do not add or remove children from the returned value, as this will 
        /// prevent them from being property mounted and unmounted.
        ///
        /// See LayoutChildren() for description of the difference between logical and physical 
        /// children.

        virtual std::vector<LvtkElement::ptr> &Children() { return children; }
        virtual const std::vector<LvtkElement::ptr> &Children() const { return children; }

        /// @brief Remove all children.
        virtual void RemoveAllChildren();

        /// @brief Get the number of direct child elements.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual size_t LayoutChildCount() const;

        /// @brief Get the specified physical child element.
        /// @param index the index of the child to return.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual const LvtkElement *LayoutChild(size_t index) const;
        virtual LvtkElement::ptr LayoutChild(size_t index);

        /// @brief Get a list of the current pyshical children. 
        /// Caution: do not add or remove children from the returned value, as this will 
        /// prevent them from being property mounted and unmounted.
        ///
        /// LayoutChildren() returns the list of physical children; Children() returns the 
        /// list of logical children. Some controls have internal elements between their 
        /// children and the element itself. Children() returns the list of logical children.
        /// LayoutChildren() includes the internal structure of the element (if any).

        virtual std::vector<LvtkElement::ptr> &LayoutChildren() { return children; }




    public:
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;
    protected:
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;

        virtual bool isContainer() const override { return true; }


    public: 
        virtual void FinalizeLayout(const LvtkRectangle &layoutClipBounds,const LvtkRectangle& screenOffest, bool clippedInLayout = false) override;
    protected:
        virtual void OnLayoutComplete() override;
        
        virtual bool FireMouseDown(LvtkMouseEventArgs&event) override;
        virtual bool FireMouseUp(LvtkMouseEventArgs&event) override;
        virtual void UpdateMouseOver(LvtkPoint mousePosition) override;
        virtual void Mount(LvtkWindow *window) override;
        virtual void Unmount(LvtkWindow *window) override;
        virtual void DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &parentBounds) override;

        std::vector<std::shared_ptr<LvtkElement>> children;
        struct  ChildInfo{
            LvtkSize measuredSize;
        };
        std::vector<ChildInfo> childInfos;
    };
}// namespace 