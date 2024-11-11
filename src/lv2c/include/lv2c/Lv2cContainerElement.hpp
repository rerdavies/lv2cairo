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

#include "Lv2cElement.hpp"

#pragma once


#pragma GCC diagnostic ignored "-Woverloaded-virtual" // GCC 13 warning.

namespace lv2c {
    class Lv2cContainerElement: public Lv2cElement {
    public:
        virtual const char* Tag() const override { return "Container";}

        using super = Lv2cElement;
        using self = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        virtual ~Lv2cContainerElement() { }



        /// @brief Add an element to the list of child elements.
        /// @param child The element to add.
        /// @param position Where to add the element.
        virtual void AddChild(std::shared_ptr<Lv2cElement> child);
        virtual void AddChild(std::shared_ptr<Lv2cElement> child,size_t position);


        /// @brief Remove an element from the list of child elements.
        /// @param element the element to remove.
        /// @returns false if the element was not currently a child.
        virtual bool RemoveChild(std::shared_ptr<Lv2cElement> element);
        /// @brief Removed the child at the specified position;
        /// @param index the position of the child to remove.
        /// @throws std::range_error if the index is invalid.
        virtual void RemoveChild(size_t index);

        /// @brief Get the child at the specified position;
        /// @param index the position of the child to return.
        /// @returns the specified child.
        /// @throws std::range_error if the index is invalid.
        //virtual Lv2cElement::ptr Child(size_t index);


        /// @brief Get the specified logical child element.
        /// @param index the index of the child to return.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual const Lv2cElement *Child(size_t index) const;
        virtual Lv2cElement::ptr Child(size_t index);

        /// @brief Get the number of direct child elements.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual size_t ChildCount() const;


        /// @brief Replace all children with the supplied list of new children.
        /// @param children the list of children with which to replace the current children.
        virtual void Children(const std::vector<Lv2cElement::ptr>&children);

        /// @brief Get a list of the current logical children. 
        /// Caution: do not add or remove children from the returned value, as this will 
        /// prevent them from being property mounted and unmounted.
        ///
        /// See LayoutChildren() for description of the difference between logical and physical 
        /// children.

        virtual std::vector<Lv2cElement::ptr> &Children() { return children; }
        virtual const std::vector<Lv2cElement::ptr> &Children() const { return children; }

        /// @brief Remove all children.
        virtual void RemoveAllChildren();

        /// @brief Get the number of direct child elements.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual size_t LayoutChildCount() const;

        /// @brief Get the specified physical child element.
        /// @param index the index of the child to return.
        // See LayoutChildren() for an explanation of logical and physical children.
        virtual const Lv2cElement *LayoutChild(size_t index) const;
        virtual Lv2cElement::ptr LayoutChild(size_t index);

        /// @brief Get a list of the current pyshical children. 
        /// Caution: do not add or remove children from the returned value, as this will 
        /// prevent them from being property mounted and unmounted.
        ///
        /// LayoutChildren() returns the list of physical children; Children() returns the 
        /// list of logical children. Some controls have internal elements between their 
        /// children and the element itself. Children() returns the list of logical children.
        /// LayoutChildren() includes the internal structure of the element (if any).

        virtual std::vector<Lv2cElement::ptr> &LayoutChildren() { return children; }




    public:
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;
    protected:
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;

        virtual bool isContainer() const override { return true; }


    public: 
        virtual void FinalizeLayout(const Lv2cRectangle &layoutClipBounds,const Lv2cRectangle& screenOffest, bool clippedInLayout = false) override;
    protected:
        virtual void OnLayoutComplete() override;


        virtual bool FireScrollWheel(Lv2cScrollWheelEventArgs &event) override;

        virtual bool FireMouseDown(Lv2cMouseEventArgs&event) override;
        virtual bool FireMouseUp(Lv2cMouseEventArgs&event) override;
        virtual void UpdateMouseOver(Lv2cPoint mousePosition) override;
        virtual void Mount(Lv2cWindow *window) override;
        virtual void Unmount(Lv2cWindow *window) override;
        virtual void DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &parentBounds) override;

        std::vector<std::shared_ptr<Lv2cElement>> children;
        struct  ChildInfo{
            Lv2cSize measuredSize;
        };
        std::vector<ChildInfo> childInfos;
    };
}// namespace 