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

#include "lv2c/Lv2cScrollContainerElement.hpp"
#include "lv2c/Lv2cScrollBarElement.hpp"
#include "lv2c/Lv2cLog.hpp"
#include <cmath>

#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/keysymdef.h>

using namespace lv2c;

Lv2cScrollContainerElement::Lv2cScrollContainerElement()
{
    this->horizontalScrollBar = Lv2cHorizontalScrollBarElement::Create();
    this->verticalScrollBar = Lv2cVerticalScrollBarElement::Create();
    AddChild(horizontalScrollBar);
    AddChild(verticalScrollBar);

    VerticalScrollOffsetProperty.Bind(verticalScrollBar->ScrollOffsetProperty);
    HorizontalScrollOffsetProperty.Bind(horizontalScrollBar->ScrollOffsetProperty);
    HorizontalScrollEnabledProperty.SetElement(this, &Lv2cScrollContainerElement::OnHorizontalScrollEnableChanged);
    VerticalScrollEnabledProperty.SetElement(this, &Lv2cScrollContainerElement::OnVerticalScrollEnableChanged);
    OnHorizontalScrollEnableChanged(HorizontalScrollEnabled());
    OnVerticalScrollEnableChanged(VerticalScrollEnabled());

    HorizontalScrollOffsetProperty.SetElement(this, &Lv2cScrollContainerElement::OnHorizontalScrollOffsetChanged);
    VerticalScrollOffsetProperty.SetElement(this, &Lv2cScrollContainerElement::OnVerticalScrollOffsetChanged);

    HorizontalDocumentSizeProperty.Bind(horizontalScrollBar->DocumentSizeProperty);
    HorizontalWindowSizeProperty.Bind(horizontalScrollBar->WindowSizeProperty);
    VerticalDocumentSizeProperty.Bind(verticalScrollBar->DocumentSizeProperty);
    VerticalWindowSizeProperty.Bind(verticalScrollBar->WindowSizeProperty);
}
Lv2cScrollContainerElement &Lv2cScrollContainerElement::Child(Lv2cElement::ptr child)
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
Lv2cElement::ptr Lv2cScrollContainerElement::Child() const
{
    return child;
}

/// Privatized Lv2cContainer methods that would be lethal for this class.
void Lv2cScrollContainerElement::AddChild(std::shared_ptr<Lv2cElement> child) { super::AddChild(child); }
bool Lv2cScrollContainerElement::RemoveChild(std::shared_ptr<Lv2cElement> element) { return super::RemoveChild(element); }
void Lv2cScrollContainerElement::RemoveChild(size_t index) { super::RemoveChild(index); }
Lv2cElement::ptr Lv2cScrollContainerElement::Child(size_t index) { return super::Child(index); }
const Lv2cElement *Lv2cScrollContainerElement::Child(size_t index) const { return super::Child(index); }
void Lv2cScrollContainerElement::Children(const std::vector<Lv2cElement::ptr> &children) { super::Children(children); }
void Lv2cScrollContainerElement::RemoveAllChildren() { super::RemoveAllChildren(); }
std::vector<Lv2cElement::ptr> &Lv2cScrollContainerElement::Children() { return super::Children(); }

void Lv2cScrollContainerElement::OnHorizontalScrollEnableChanged(bool value)
{
    horizontalScrollBar->Style().Visibility(HorizontalScrollEnabled() ? Lv2cVisibility::Visible : Lv2cVisibility::Collapsed);
    InvalidateLayout();
}
void Lv2cScrollContainerElement::OnVerticalScrollEnableChanged(bool value)
{
    verticalScrollBar->Style().Visibility(VerticalScrollEnabled() ? Lv2cVisibility::Visible : Lv2cVisibility::Collapsed);
    InvalidateLayout();
}

Lv2cSize Lv2cScrollContainerElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
{
    double width = clientConstraint.Width();
    double height = clientConstraint.Height();
    if (HorizontalScrollEnabled() && VerticalScrollEnabled())
    {
        // pad the scrollbars so they don't overlap.
        {
            Lv2cSize constraint = clientConstraint;
            Lv2cSize available = clientAvailable;
            double scrollbarExtra = horizontalScrollBar->Style().Height().PixelValue();
            if (constraint.Height() != 0)
            {
                constraint.Height(constraint.Height() - scrollbarExtra);
            }
            if (available.Height() != 0)
            {
                available.Height(available.Height() - scrollbarExtra);
            }
            verticalScrollBar->Measure(constraint, available, context);
        }
        // pad the scrollbars so they don't overlap.
        {
            Lv2cSize constraint = clientConstraint;
            Lv2cSize available = clientAvailable;
            double scrollbarExtra = verticalScrollBar->Style().Height().PixelValue();
            if (constraint.Height() != 0)
            {
                constraint.Width(constraint.Width() - scrollbarExtra);
            }
            if (available.Width() != 0)
            {
                available.Width(available.Width() - scrollbarExtra);
            }
            horizontalScrollBar->Measure(constraint, available, context);
        }
    }
    else if (HorizontalScrollEnabled())
    {
        horizontalScrollBar->Measure(clientConstraint, clientAvailable, context);
    }
    else if (VerticalScrollEnabled())
    {
        verticalScrollBar->Measure(clientConstraint, clientAvailable, context);
    }
    if (child)
    {
        Lv2cSize constraint(0, 0);
        Lv2cSize available(3E15, 3E15); // unimaginably large.
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
        child->Measure(constraint, available, context);
    }
    if (width == 0)
    {
        width = 50;
        LogError("Lv2cScrollContainer has unconstrained width. Can't decide how wide it should be.");
    }
    if (height == 0)
    {
        height = 50;
        LogError("Lv2cScrollContainer has unconstrained height. Can't decide how wide it should be.");
    }
    return Lv2cSize(width, height);
}

Lv2cSize Lv2cScrollContainerElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    Lv2cRectangle marginRect{0, 0, available.Width(), available.Height()};

    Lv2cRectangle borderRect = this->removeThickness(marginRect, Style().Margin());
    Lv2cRectangle paddingRect = this->removeThickness(borderRect, Style().BorderWidth());
    Lv2cRectangle clientRect = this->removeThickness(paddingRect, Style().Padding());

    if (HorizontalScrollEnabled())
    {
        Lv2cSize measured = horizontalScrollBar->MeasuredSize();
        measured = horizontalScrollBar->Arrange(horizontalScrollBar->MeasuredSize(), context);

        Lv2cRectangle rectangle{
            0, clientRect.Height() - measured.Height(),
            measured.Width(), measured.Height()};
        horizontalScrollBar->Layout(rectangle);
    }
    if (VerticalScrollEnabled())
    {
        Lv2cSize measured = verticalScrollBar->MeasuredSize();
        measured = verticalScrollBar->Arrange(verticalScrollBar->MeasuredSize(), context);

        Lv2cRectangle rectangle{
            clientRect.Width() - measured.Width(), 0,
            measured.Width(), measured.Height()};
        verticalScrollBar->Layout(rectangle);
    }

    if (child)
    {
        Lv2cSize measured = child->MeasuredSize();
        measured = child->Arrange(measured, context);
        Lv2cRectangle rectangle{
            -HorizontalScrollOffset(), -VerticalScrollOffset(),
            measured.Width(), measured.Height()};
        child->Layout(rectangle);
        this->childSize = measured;
    }
    else
    {
        this->childSize = Lv2cSize(0, 0);
    }
    return available;
}

void Lv2cScrollContainerElement::FinalizeLayout(const Lv2cRectangle &layoutClipRect, const Lv2cRectangle &screenOffset, bool clippedInLayout)
{
    this->savedClippedInLayout = clippedInLayout;
    this->savedLayoutClipRect = layoutClipRect;
    super::FinalizeLayout(layoutClipRect, screenOffset, clippedInLayout);
    HorizontalWindowSize(this->ClientSize().Width());
    VerticalWindowSize(this->ClientSize().Height());
    if (child)
    {
        HorizontalDocumentSize(this->childSize.Width());
        VerticalDocumentSize(this->childSize.Height());
    }
    else
    {
        HorizontalDocumentSize(0);
        VerticalDocumentSize(0);
    }

    // warning: these trigger partial scroll layout. Maybe PostDelayed?
    if (HorizontalScrollOffset() > HorizontalDocumentSize() - HorizontalWindowSize())
    {
        HorizontalScrollOffset(std::max(0.0, HorizontalDocumentSize() - HorizontalWindowSize()));
    }
    if (VerticalScrollOffset() > VerticalDocumentSize() - VerticalWindowSize())
    {
        VerticalScrollOffset(std::max(0.0, VerticalDocumentSize() - VerticalWindowSize()));
    }
    MaximumHorizontalScrollOffset(std::max(0.0, HorizontalDocumentSize() - HorizontalWindowSize()));
    MaximumVerticalScrollOffset(std::max(0.0, VerticalDocumentSize() - VerticalWindowSize()));
}

void Lv2cScrollContainerElement::OnHorizontalScrollOffsetChanged(double value)
{
    RedoFinalLayout();
}
void Lv2cScrollContainerElement::OnVerticalScrollOffsetChanged(double value)
{
    RedoFinalLayout();
}

void Lv2cScrollContainerElement::RedoFinalLayout()
{
    if (child)
    {
        // update the child's layout.
        Lv2cRectangle layoutRect{-HorizontalScrollOffset(), -VerticalScrollOffset(), this->childSize.Width(), this->childSize.Height()};
        child->Layout(layoutRect);

        // recompute visual rects for this and all children
        this->FinalizeLayout(this->savedLayoutClipRect, Parent()->ScreenBounds(), this->savedClippedInLayout);
    }
}

bool Lv2cScrollContainerElement::ClipChildren() const
{
    return true;
}

bool Lv2cScrollContainerElement::OnScrollWheel(Lv2cScrollWheelEventArgs &event)
{
    constexpr double SCROLL_AMOUNT = 24;

    switch (event.scrollDirection)
    {
    case Lv2cScrollDirection::Left:
    {
        if (this->HorizontalScrollEnabled())
        {
            double newValue = HorizontalScrollOffset() - SCROLL_AMOUNT;
            if (newValue < 0)
            {
                newValue = 0;
            }
            HorizontalScrollOffset(newValue);
            return true;
        }
        break;
    }
    case Lv2cScrollDirection::Right:
    {
        if (this->HorizontalScrollEnabled())
        {
            double newValue = HorizontalScrollOffset() + SCROLL_AMOUNT;
            if (newValue > MaximumHorizontalScrollOffset())
            {
                newValue = MaximumHorizontalScrollOffset();
            }
            HorizontalScrollOffset(newValue);
            return true;
        }
        break;
    }

    case Lv2cScrollDirection::Up:
    {
        if (this->VerticalScrollEnabled())
        {
            double newValue = VerticalScrollOffset() - SCROLL_AMOUNT;
            if (newValue < 0)
            {
                newValue = 0;
            }
            VerticalScrollOffset(newValue);
            return true;
        }
        break;
    }
    case Lv2cScrollDirection::Down:
        if (this->VerticalScrollEnabled())
        {
            double newValue = VerticalScrollOffset() + SCROLL_AMOUNT;
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

bool Lv2cScrollContainerElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
{
    if (event.keysymValid)
    {
        if (!VerticalScrollEnabled())
        {
            return false;
        }
        if (event.modifierState == ModifierState::Empty)
        {
        constexpr double LINE_AMOUNT = 16;
        double pageAmount = std::floor(ClientBounds().Height()-LINE_AMOUNT*2);
        if (pageAmount < LINE_AMOUNT) pageAmount = LINE_AMOUNT;
        double scrollDistance = 0;
        switch (event.keysym)
        {
        case XK_Up:
        case XK_KP_Up:
            scrollDistance = -LINE_AMOUNT;
            break;
        case XK_Down:
        case XK_KP_Down:
            scrollDistance = LINE_AMOUNT;
            break;
        case XK_Page_Up:
        case XK_KP_Page_Up:
            scrollDistance = -pageAmount;
            break;
        case XK_Page_Down:
        case XK_KP_Page_Down:
            scrollDistance = pageAmount;
            break;
        case XK_Home:
        case XK_KP_Home:
            scrollDistance = -MaximumVerticalScrollOffset();
            break;
        case XK_End:
        case XK_KP_End:
            scrollDistance = MaximumVerticalScrollOffset();
            break;
        default:
            break;
        }
        double scroll = VerticalScrollOffset() + scrollDistance;
        if (scroll < 0) scroll = 0;
        if (scroll > MaximumVerticalScrollOffset())
        {
            scroll = MaximumVerticalScrollOffset();
        }
        VerticalScrollOffset(scroll);
        return true;
    }
    } else if (event.modifierState == ModifierState::Shift)
    {
        if (!HorizontalScrollEnabled())
        {
            return false;
        }
        constexpr double LINE_AMOUNT = 16;
        double pageAmount = std::floor(ClientBounds().Width()-LINE_AMOUNT*2);
        if (pageAmount < LINE_AMOUNT) pageAmount = LINE_AMOUNT;
        double scrollDistance = 0;
        switch (event.keysym)
        {
        case XK_Up:
        case XK_KP_Up:
            scrollDistance = -LINE_AMOUNT;
            break;
        case XK_Down:
        case XK_KP_Down:
            scrollDistance = LINE_AMOUNT;
            break;
        case XK_Page_Up:
        case XK_KP_Page_Up:
            scrollDistance = -pageAmount;
            break;
        case XK_Page_Down:
        case XK_KP_Page_Down:
            scrollDistance = pageAmount;
            break;
        case XK_Home:
        case XK_KP_Home:
            scrollDistance = -MaximumVerticalScrollOffset();
            break;
        case XK_End:
        case XK_KP_End:
            scrollDistance = MaximumVerticalScrollOffset();
            break;
        default:
            break;
        }
        double scroll = HorizontalScrollOffset() + scrollDistance;
        if (scroll < 0) scroll = 0;
        if (scroll > MaximumHorizontalScrollOffset())
        {
            scroll = MaximumHorizontalScrollOffset();
        }
        HorizontalScrollOffset(scroll);
        return true;

    }
    return false;
}
bool Lv2cScrollContainerElement::WantsFocus() const
{
    return wantsFocus;
}
Lv2cScrollContainerElement &Lv2cScrollContainerElement::WantsFocus(bool value)
{
    this->wantsFocus = value;
    return *this;
}
