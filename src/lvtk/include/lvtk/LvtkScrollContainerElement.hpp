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

#include "LvtkContainerElement.hpp"
#include "LvtkBindingProperty.hpp"

namespace lvtk
{
    class LvtkScrollBarElement;

    /// @brief A container that scrolls it's child.
    /// LvtkScrollContainerElement accepts one child element. Use the Child() getter and setter to set and get the child element.
    ///
    /// A LvtkScrollContainer can have a horizontal scrollbar, or a vertical scrollbar, or both. Horizontal and 
    /// vertical scrollbars can be enabled or disabled. When enabled, a scroll indicator is displayed in the last 
    /// 4 pixels of the margin of the LvtkScrollContainer, and the last 16 pixels of the margin are used to 
    /// interact with the scrollbars using the mouse. By design, the child element displays underneath both the active 
    /// scroll control, and the indicator, so it is prudent to include at least 16 pixels of margin or padding
    /// on the bottom and/or left of any control that is used as a child of a LvtkScrollContainer (or at least 
    /// not place any active content in that area). Most user interfaces add a 16- or 24-point margin to the 
    /// left and right-hand side, so this isn't generally a problem. Just remember to add padding at the bottom 
    /// to ensure that the child's active content doesn't get displayed under a horizontal scrollbar.
    ///
    /// Children of LvtkScrollContainers have constrained horizontal layout if the horizontal scrollbar is disabled
    /// A child with 100% width will align with the right border of the lvtk element, and unconstrained horizontal 
    /// layout if the horizontal scrollbar is enabled. Similarly, children have constrained or unconstrained vertical
    /// layout if the vertical scrollbar is disabled or enabled, respectively.

    class LvtkScrollContainerElement : public LvtkContainerElement {
    public:
        using self = LvtkScrollContainerElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkScrollContainerElement();        


        LvtkScrollContainerElement& Child(LvtkElement::ptr child);
        LvtkElement::ptr Child() const;

        BINDING_PROPERTY(HorizontalScrollEnabled, bool, false)
        BINDING_PROPERTY(VerticalScrollEnabled, bool, true)

        /// @brief The Horizontal scroll offset.
        /// You can set this property to change the scroll offset of the child control
        BINDING_PROPERTY(HorizontalScrollOffset, double, 0)
        /// @brief The Horizontal scroll offset.
        /// You can set this property to change the scroll offset of the child control. 
        BINDING_PROPERTY(VerticalScrollOffset, double, 0)

        /// @brief The Maximum allowed value for HorizontalScrollOffset (read-only).
        /// This property is calculated at after layout to allow external 
        /// components to set appropriate values for HorizontalScrollOffsets.
        ///
        /// The value is not valid until a layout pass has been completed. 
        /// The calculated value is posted after the layout pass completes.
        /// Be careful not to create layout storms if you watch or bind to this property. 
        /// 
        BINDING_PROPERTY(MaximumHorizontalScrollOffset,double, -1)

        /// @brief The Maximum allowed value for HorizontalScrollOffset (read-only).
        /// This property is calculated at after layout to allow external 
        /// components to set appropriate values for HorizontalScrollOffsets.
        /// The value depends on both the layout size of the LvtkScrollPropertyElement,
        /// and on the layout size of its' child.
        ///
        /// If HorizontalScrollOffset exceeds the calculated value of MaximumHorizontalScrollOffset
        /// at the end of a layout pass, the value of HorizontalScrollOffset is adjusted accordingly.
        ///
        /// The value is not valid until a layout pass has been completed. 
        /// The calculated value is posted after the layout pass completes.
        /// Be careful not to create layout storms if you watch or bind to this property. 
        /// 
        BINDING_PROPERTY(MaximumVerticalScrollOffset,double, -1)


        ///@brief Scroll a child element into view. 
        ///@param element The element to scroll into view.
        ///@param surroundingSpace The space to provide between the scrolled element, and the borders of the LvtkScrollContainerELement.
        ///Adjust the Horizontal and/or Vertical scroll offsets so the seleted item is 
        ///visible. Has no effect if the element is not a descendent of LvtkScrollContainerElement.
        ///
        ///The minimum adjustment to scroll offset is made to satisfy the requested surroundingSpace constraint. Set 
        ///the left, top, right, or bottom members of surroundingSpace to -1 to disable the space request on that edge.
        void ScrollIntoView(LvtkElement *element, LvtkThickness surroundingSpace);

    protected:
        virtual bool ClipChildren() const override;

        virtual void OnHorizontalScrollEnableChanged(bool value);
        virtual void OnVerticalScrollEnableChanged(bool value);
        virtual void OnHorizontalScrollOffsetChanged(double value);
        virtual void OnVerticalScrollOffsetChanged(double value);
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;
    public:
        virtual LvtkSize Arrange(LvtkSize available, LvtkDrawingContext &context) override;
        virtual void FinalizeLayout(const LvtkRectangle&layoutClipRect,const LvtkRectangle& screenOffest, bool clippedInLayout = false) override;



    private:
        BINDING_PROPERTY(HorizontalWindowSize,double,0);
        BINDING_PROPERTY(HorizontalDocumentSize, double,0);
        BINDING_PROPERTY(VerticalWindowSize,double, 0);
        BINDING_PROPERTY(VerticalDocumentSize,double,0);
        LvtkElement::ptr child;
        LvtkSize childSize;
        std::shared_ptr<LvtkScrollBarElement> horizontalScrollBar;
        std::shared_ptr<LvtkScrollBarElement> verticalScrollBar;
        bool savedClippedInLayout = false;

    private: 
        LvtkRectangle savedLayoutClipRect;
        void RedoFinalLayout();
        // Hide these methods.
        void AddChild(std::shared_ptr<LvtkElement> child) override;
        bool RemoveChild(std::shared_ptr<LvtkElement> element) override;
        void RemoveChild(size_t index) override;
        LvtkElement::ptr Child(size_t index) override;
        const LvtkElement *Child(size_t index) const override;
        void Children(const std::vector<LvtkElement::ptr>&children) override;
        void RemoveAllChildren() override; 
        std::vector<LvtkElement::ptr> &Children() override;
 
    };
}
