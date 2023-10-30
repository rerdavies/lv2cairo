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
#include "Lv2cBindingProperty.hpp"

namespace lv2c
{
    class Lv2cScrollBarElement;

    /// @brief A container that scrolls it's child.
    /// Lv2cScrollContainerElement accepts one child element. Use the Child() getter and setter to set and get the child element.
    ///
    /// A Lv2cScrollContainer can have a horizontal scrollbar, or a vertical scrollbar, or both. Horizontal and 
    /// vertical scrollbars can be enabled or disabled. When enabled, a scroll indicator is displayed in the last 
    /// 4 pixels of the margin of the Lv2cScrollContainer, and the last 16 pixels of the margin are used to 
    /// interact with the scrollbars using the mouse. By design, the child element displays underneath both the active 
    /// scroll control, and the indicator, so it is prudent to include at least 16 pixels of margin or padding
    /// on the bottom and/or left of any control that is used as a child of a Lv2cScrollContainer (or at least 
    /// not place any active content in that area). Most user interfaces add a 16- or 24-point margin to the 
    /// left and right-hand side, so this isn't generally a problem. Just remember to add padding at the bottom 
    /// to ensure that the child's active content doesn't get displayed under a horizontal scrollbar.
    ///
    /// Children of Lv2cScrollContainers have constrained horizontal layout if the horizontal scrollbar is disabled
    /// A child with 100% width will align with the right border of the lv2cairo element, and unconstrained horizontal 
    /// layout if the horizontal scrollbar is enabled. Similarly, children have constrained or unconstrained vertical
    /// layout if the vertical scrollbar is disabled or enabled, respectively.

    class Lv2cScrollContainerElement : public Lv2cContainerElement {
    public:
        using self = Lv2cScrollContainerElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cScrollContainerElement();        


        Lv2cScrollContainerElement& Child(Lv2cElement::ptr child);
        Lv2cElement::ptr Child() const;

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
        /// The value depends on both the layout size of the Lv2cScrollPropertyElement,
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
        ///@param surroundingSpace The space to provide between the scrolled element, and the borders of the Lv2cScrollContainerELement.
        ///Adjust the Horizontal and/or Vertical scroll offsets so the seleted item is 
        ///visible. Has no effect if the element is not a descendent of Lv2cScrollContainerElement.
        ///
        ///The minimum adjustment to scroll offset is made to satisfy the requested surroundingSpace constraint. Set 
        ///the left, top, right, or bottom members of surroundingSpace to -1 to disable the space request on that edge.
        void ScrollIntoView(Lv2cElement *element, Lv2cThickness surroundingSpace);


        virtual bool WantsFocus() const override;
        virtual Lv2cScrollContainerElement& WantsFocus(bool value);

    protected:
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs&event) override;

        virtual bool OnScrollWheel(Lv2cScrollWheelEventArgs &event) override;

        virtual bool ClipChildren() const override;

        virtual void OnHorizontalScrollEnableChanged(bool value);
        virtual void OnVerticalScrollEnableChanged(bool value);
        virtual void OnHorizontalScrollOffsetChanged(double value);
        virtual void OnVerticalScrollOffsetChanged(double value);
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;
    public:
        virtual Lv2cSize Arrange(Lv2cSize available, Lv2cDrawingContext &context) override;
        virtual void FinalizeLayout(const Lv2cRectangle&layoutClipRect,const Lv2cRectangle& screenOffest, bool clippedInLayout = false) override;



    private:
        BINDING_PROPERTY(HorizontalWindowSize,double,0);
        BINDING_PROPERTY(HorizontalDocumentSize, double,0);
        BINDING_PROPERTY(VerticalWindowSize,double, 0);
        BINDING_PROPERTY(VerticalDocumentSize,double,0);
        Lv2cElement::ptr child;
        Lv2cSize childSize;
        std::shared_ptr<Lv2cScrollBarElement> horizontalScrollBar;
        std::shared_ptr<Lv2cScrollBarElement> verticalScrollBar;
        bool savedClippedInLayout = false;

    private: 
        bool wantsFocus = false;
        Lv2cRectangle savedLayoutClipRect;
        void RedoFinalLayout();
        // Hide these methods.
        void AddChild(std::shared_ptr<Lv2cElement> child) override;
        bool RemoveChild(std::shared_ptr<Lv2cElement> element) override;
        void RemoveChild(size_t index) override;
        Lv2cElement::ptr Child(size_t index) override;
        const Lv2cElement *Child(size_t index) const override;
        void Children(const std::vector<Lv2cElement::ptr>&children) override;
        void RemoveAllChildren() override; 
        std::vector<Lv2cElement::ptr> &Children() override;
 
    };
}
