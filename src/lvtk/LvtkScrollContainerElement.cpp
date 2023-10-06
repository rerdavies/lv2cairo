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

#include "lvtk/LvtkScrollContainerElement.hpp"
#include "lvtk/LvtkScrollBarElement.hpp"
#include "lvtk/LvtkLog.hpp"
#include <cmath>

using namespace lvtk;

LvtkScrollContainerElement::LvtkScrollContainerElement()
{
    this->horizontalScrollBar = LvtkHorizontalScrollBarElement::Create();
    this->verticalScrollBar  = LvtkVerticalScrollBarElement::Create();
    AddChild(horizontalScrollBar);
    AddChild(verticalScrollBar);

    VerticalScrollOffsetProperty.Bind(verticalScrollBar->ScrollOffsetProperty);
    HorizontalScrollOffsetProperty.Bind(horizontalScrollBar->ScrollOffsetProperty);
    HorizontalScrollEnabledProperty.SetElement(this,&LvtkScrollContainerElement::OnHorizontalScrollEnableChanged);
    VerticalScrollEnabledProperty.SetElement(this,&LvtkScrollContainerElement::OnVerticalScrollEnableChanged);
    OnHorizontalScrollEnableChanged(HorizontalScrollEnabled());
    OnVerticalScrollEnableChanged(VerticalScrollEnabled());

    HorizontalScrollOffsetProperty.SetElement(this,&LvtkScrollContainerElement::OnHorizontalScrollOffsetChanged);
    VerticalScrollOffsetProperty.SetElement(this,&LvtkScrollContainerElement::OnVerticalScrollOffsetChanged);

    HorizontalDocumentSizeProperty.Bind(horizontalScrollBar->DocumentSizeProperty);
    HorizontalWindowSizeProperty.Bind(horizontalScrollBar->WindowSizeProperty);
    VerticalDocumentSizeProperty.Bind(verticalScrollBar->DocumentSizeProperty);
    VerticalWindowSizeProperty.Bind(verticalScrollBar->WindowSizeProperty);



}
LvtkScrollContainerElement &LvtkScrollContainerElement::Child(LvtkElement::ptr child)
{
    if (this->child)
    {
        super::RemoveChild(0);
        this->child = nullptr;
    }
    this->child = child;
    if (this->child)
    {
        super::AddChild(this->child, 0);
    }
    return *this;
}
LvtkElement::ptr LvtkScrollContainerElement::Child() const
{
    return child;
}

/// Privatized LvtkContainer methods that would be lethal for this class.
void LvtkScrollContainerElement::AddChild(std::shared_ptr<LvtkElement> child) { super::AddChild(child); }
bool LvtkScrollContainerElement::RemoveChild(std::shared_ptr<LvtkElement> element) { return super::RemoveChild(element); }
void LvtkScrollContainerElement::RemoveChild(size_t index) { super::RemoveChild(index); }
LvtkElement::ptr LvtkScrollContainerElement::Child(size_t index) { return super::Child(index); }
const LvtkElement *LvtkScrollContainerElement::Child(size_t index) const { return super::Child(index); }
void LvtkScrollContainerElement::Children(const std::vector<LvtkElement::ptr> &children) { super::Children(children); }
void LvtkScrollContainerElement::RemoveAllChildren() { super::RemoveAllChildren(); }
std::vector<LvtkElement::ptr> &LvtkScrollContainerElement::Children() { return super::Children(); }


void LvtkScrollContainerElement::OnHorizontalScrollEnableChanged(bool value)
{
    horizontalScrollBar->Style().
        Visibility(HorizontalScrollEnabled()? LvtkVisibility::Visible: LvtkVisibility::Collapsed);
    InvalidateLayout();

}
void LvtkScrollContainerElement::OnVerticalScrollEnableChanged(bool value)
{
    verticalScrollBar->Style().
        Visibility(VerticalScrollEnabled()? LvtkVisibility::Visible: LvtkVisibility::Collapsed);
    InvalidateLayout();

}

LvtkSize LvtkScrollContainerElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context)
{
    double width = clientConstraint.Width();
    double height = clientConstraint.Height();
    if (HorizontalScrollEnabled() && VerticalScrollEnabled())
    {
        // pad the scrollbars so they don't overlap.
        {
            LvtkSize constraint = clientConstraint;
            LvtkSize available = clientAvailable;
            double scrollbarExtra = horizontalScrollBar->Style().Height().PixelValue();
            if (constraint.Height() != 0)
            {
                constraint.Height(constraint.Height()-scrollbarExtra);
            }
            if (available.Height() != 0)
            {
                available.Height(available.Height()-scrollbarExtra);
            }
            verticalScrollBar->Measure(constraint,available,context);
        }
        // pad the scrollbars so they don't overlap.
        {
            LvtkSize constraint = clientConstraint;
            LvtkSize available = clientAvailable;
            double scrollbarExtra = verticalScrollBar->Style().Height().PixelValue();
            if (constraint.Height() != 0)
            {
                constraint.Width(constraint.Width()-scrollbarExtra);
            }
            if (available.Width() != 0)
            {
                available.Width(available.Width()-scrollbarExtra);
            }
            horizontalScrollBar->Measure(constraint,available,context);
        } 
    }else if (HorizontalScrollEnabled())
    {
        horizontalScrollBar->Measure(clientConstraint, clientAvailable,context);
    } else if (VerticalScrollEnabled())
    {
        verticalScrollBar->Measure(clientConstraint, clientAvailable,context);
    }
    if (child)
    {
        LvtkSize constraint(0,0);
        LvtkSize available(3E15, 3E15); // unimaginably large.
        if (!HorizontalScrollEnabled())
        {
            constraint.Width(clientConstraint.Width());
            available.Width(clientAvailable.Width());
        }
        if (!VerticalScrollEnabled())
        {
            constraint.Height(clientConstraint.Height());
            constraint.Width(clientAvailable.Width());
        }
        child->Measure(constraint,available,context);
    }
    if (width == 0) 
    {
        width = 50;
        LogError("LvtkScrollContainer has unconstrained width. Can't decide how wide it should be.");
    }
    if (height == 0) 
    {
        height = 50;
        LogError("LvtkScrollContainer has unconstrained height. Can't decide how wide it should be.");
    }
    return LvtkSize(width,height);    
}

LvtkSize LvtkScrollContainerElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    LvtkRectangle marginRect{0, 0, available.Width(), available.Height()};

    LvtkRectangle borderRect = this->removeThickness(marginRect, Style().Margin());
    LvtkRectangle paddingRect = this->removeThickness(borderRect, Style().BorderWidth());
    LvtkRectangle clientRect = this->removeThickness(paddingRect, Style().Padding());

    if (HorizontalScrollEnabled())
    {
        LvtkSize measured = horizontalScrollBar->MeasuredSize();
        measured = horizontalScrollBar->Arrange(horizontalScrollBar->MeasuredSize(), context);

        LvtkRectangle rectangle { 
            0, clientRect.Height()-measured.Height(),
            measured.Width(), measured.Height()};
        horizontalScrollBar->Layout(rectangle);
    }
    if (VerticalScrollEnabled())
    {
        LvtkSize measured = verticalScrollBar->MeasuredSize();
        measured = verticalScrollBar->Arrange(verticalScrollBar->MeasuredSize(), context);

        LvtkRectangle rectangle { 
            clientRect.Width()-measured.Width(), 0,
            measured.Width(), measured.Height()};
        verticalScrollBar->Layout(rectangle);
    }

    if (child)
    {
        LvtkSize measured = child->MeasuredSize();
        measured = child->Arrange(measured,context);
        LvtkRectangle rectangle {
            -HorizontalScrollOffset(),-VerticalScrollOffset(),
            measured.Width(), measured.Height()
        };
        child->Layout(rectangle);
        this->childSize = measured;
    } else {
        this->childSize = LvtkSize(0,0);
    }
    return available;
}



void LvtkScrollContainerElement::FinalizeLayout(const LvtkRectangle&layoutClipRect,const LvtkRectangle& screenOffset, bool clippedInLayout )
{
    this->savedClippedInLayout = clippedInLayout;
    this->savedLayoutClipRect = layoutClipRect;
    super::FinalizeLayout(layoutClipRect,screenOffset,clippedInLayout);
    HorizontalWindowSize(this->ClientSize().Width());
    VerticalWindowSize(this->ClientSize().Height());
    if (child)
    {
        HorizontalDocumentSize(this->childSize.Width());
        VerticalDocumentSize(this->childSize.Height());
    } else {
        HorizontalDocumentSize(0);
        VerticalDocumentSize(0);
    }

    // warning: these trigger partial scroll layout. Maybe PostDelayed?
    if (HorizontalScrollOffset() > HorizontalDocumentSize()-HorizontalWindowSize())
    {
        HorizontalScrollOffset(std::max(0.0,HorizontalDocumentSize()-HorizontalWindowSize()));
    }
    if (VerticalScrollOffset() > VerticalDocumentSize()-VerticalWindowSize())
    {
        VerticalScrollOffset(std::max(0.0,VerticalDocumentSize()-VerticalWindowSize()));
    }
    MaximumHorizontalScrollOffset(std::max(0.0,HorizontalDocumentSize()-HorizontalWindowSize()));
    MaximumVerticalScrollOffset(std::max(0.0,VerticalDocumentSize()-VerticalWindowSize()));

}

void LvtkScrollContainerElement::OnHorizontalScrollOffsetChanged(double value)
{
    RedoFinalLayout();
}
void LvtkScrollContainerElement::OnVerticalScrollOffsetChanged(double value)
{
    RedoFinalLayout();
}

void LvtkScrollContainerElement::RedoFinalLayout()
{
    if (child)
    {
        // update the child's layout.
        LvtkRectangle layoutRect { -HorizontalScrollOffset(), -VerticalScrollOffset(), this->childSize.Width(),this->childSize.Height()};
        child->Layout(layoutRect);
        
        // recompute visual rects for this and all children
        this->FinalizeLayout(this->savedLayoutClipRect,Parent()->ScreenBounds(),this->savedClippedInLayout);


    }
}


bool LvtkScrollContainerElement::ClipChildren() const
{
    return true;
}

bool LvtkScrollContainerElement::OnScrollWheel(LvtkScrollWheelEventArgs &event) {
    constexpr double SCROLL_AMOUNT = 24;

    switch (event.scrollDirection)
    {
    case LvtkScrollDirection::Left:
    {
        if (this->HorizontalScrollEnabled())
        {
            double newValue = HorizontalScrollOffset()-SCROLL_AMOUNT;
            if (newValue < 0)
            {
                newValue = 0;
            }
            HorizontalScrollOffset(newValue);
            return true;
        }
        break;
    }
    case LvtkScrollDirection::Right:
    {
        if (this->HorizontalScrollEnabled())
        {
            double newValue = HorizontalScrollOffset() +SCROLL_AMOUNT;
            if (newValue > MaximumHorizontalScrollOffset())
            {
                newValue = MaximumHorizontalScrollOffset();
            }
            HorizontalScrollOffset(newValue);
            return true;
        }
        break;
    }

    case LvtkScrollDirection::Up:
    {
        if (this->VerticalScrollEnabled())
        {
            double newValue = VerticalScrollOffset()-SCROLL_AMOUNT;
            if (newValue < 0)
            {
                newValue = 0;
            }
            VerticalScrollOffset(newValue);
            return true;
        }
        break;
    }
    case LvtkScrollDirection::Down:
        if (this->VerticalScrollEnabled())
        {
            double newValue = VerticalScrollOffset()+SCROLL_AMOUNT;
            if (newValue > this->MaximumVerticalScrollOffset())
            {
                newValue = MaximumVerticalScrollOffset();
            }
            VerticalScrollOffset(newValue);
            return true;
        }
        break;
        
    }
    return false;
}

