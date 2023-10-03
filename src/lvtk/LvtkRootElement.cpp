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

#include "lvtk/LvtkRootElement.hpp"
#include "lvtk/LvtkWindow.hpp"

#define XK_MISCELLANY
#include <X11/keysymdef.h>

using namespace lvtk;

void LvtkRootElement::AddChildEx(
    ChildType childType,
    LvtkElement::ptr child,
    double zOrder,
    LvtkElement *anchor,
    std::function<void(void)> &&onClose)
{
    ChildInfo childInfo{childType, child, zOrder, anchor, std::move(onClose)};

    size_t position = 0;
    while (position < childInfos.size() && childInfos[position].zOrder <= zOrder)
    {
        ++position;
    }
    childInfos.insert(childInfos.begin() + position, std::move(childInfo));
    super::AddChild(child, position);
    InvalidateLayout();
}

void LvtkRootElement::AddChild(std::shared_ptr<LvtkElement> child)
{
    AddChildEx(ChildType::Normal, child, 0, nullptr, []() {});
}
bool LvtkRootElement::RemoveChild(std::shared_ptr<LvtkElement> element)
{
    Window()->Invalidate(element->ScreenBounds());
    for (auto i = childInfos.begin(); i != childInfos.end(); ++i)
    {
        if (i->child.get() == element.get())
        {
            i->onClose();
            childInfos.erase(i);
            break;
        }
    }

    return super::RemoveChild(element);
}
void LvtkRootElement::RemoveChild(size_t index)
{
    Window()->Invalidate(childInfos[index].child->ScreenBounds());
    childInfos[index].onClose();
    childInfos.erase(childInfos.begin() + index);
    super::RemoveChild(index);
}

void LvtkRootElement::UpdateMouseOver(LvtkPoint mousePosition)
{
    LvtkPoint currentMousePosition = mousePosition;
    LvtkPoint noPoint{-10000, -1000};
    if (childInfos.size() != 0)
    {
        for (int64_t i = childInfos.size() - 1; i >= 0; --i)
        {
            ChildInfo &childInfo = childInfos[i];
            childInfo.child->UpdateMouseOver(currentMousePosition);
            switch (childInfo.childType)
            {
            case ChildType::Dialog:
            case ChildType::Popup:
            default:
                currentMousePosition = noPoint; // no mouse-over for remaining windows.
                break;
            case ChildType::ModalDialog:
                if (childInfo.child->screenBorderBounds.Contains(currentMousePosition))
                {
                    currentMousePosition = noPoint;
                }
                break;
            case ChildType::Normal:
                currentMousePosition = noPoint;
                break;
            }
        }
    }
}

bool LvtkRootElement::FireMouseDown(LvtkMouseEventArgs &event)
{
    for (int64_t i = childInfos.size() - 1; i >= 0; --i)
    {
        ChildInfo &childInfo = childInfos[i];
        if (childInfo.child->FireMouseDown(event))
        {
            return true;
        }
        switch (childInfo.childType)
        {
        case ChildType::Normal:
        case ChildType::ModalDialog:
            return false;
        case ChildType::Dialog:
            break;
        case ChildType::Popup:
            RemoveChild(i);
            return true;
        }
    }
    return false;
}

bool LvtkRootElement::FireKeyDown(const LvtkKeyboardEventArgs &event)
{
    for (size_t i = childInfos.size() - 1; i >= 0; --i)
    {
        ChildInfo &childInfo = childInfos[i];
        if (childInfo.child->FireKeyDown(event))
        {
            return true;
        }
        switch (childInfo.childType)
        {
        case ChildType::Normal:
        case ChildType::ModalDialog:
            return true;
        case ChildType::Dialog:
            break;
        case ChildType::Popup:
            HandlePopupKeys(childInfo.child, event);
            return true;
        }
    }
    return false;
}

bool LvtkRootElement::HandlePopupKeys(LvtkElement::ptr child, const LvtkKeyboardEventArgs &event)
{
    if (event.keysymValid)
    {
        switch (event.keysym)
        {
        case XK_Escape:
        case XK_Cancel:
            RemoveChild(child);
            return true;
        }
    }
    return false;
}

void LvtkRootElement::AddPopup(std::shared_ptr<LvtkElement> child, LvtkElement *anchor, std::function<void(void)> &&onClose)
{
    AddChildEx(ChildType::Popup, child, 100.0, anchor, std::move(onClose));
}

LvtkSize LvtkRootElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    for (auto &childInfo : childInfos)
    {

        auto child = childInfo.child;

        LvtkSize measure = child->MeasuredSize();
        child->Arrange(child->MeasuredSize(), context);
        double childLeft = -1, childRight = -1, childTop = -1, childBottom = -1;

        switch (child->Style().HorizontalAlignment())
        {
        case LvtkAlignment::Start:
            childLeft = 0;
            childRight = measure.Width();
            break;
        case LvtkAlignment::End:
            childRight = available.Width();
            childLeft = childRight - measure.Width();
            break;
        case LvtkAlignment::Center:
        {
            double extra = (available.Width() - measure.Width()) / 2;
            childLeft = extra;
            childRight = measure.Width();
            break;
        }
        case LvtkAlignment::Stretch:
            childLeft = 0;
            childRight = available.Width();
        }
        switch (child->Style().VerticalAlignment())
        {
        case LvtkAlignment::Start:
            childTop = 0;
            childBottom = measure.Height();
            break;
        case LvtkAlignment::End:
            childBottom = available.Height();
            childTop = childBottom - measure.Height();
            break;
        case LvtkAlignment::Center:
        {
            childTop = (available.Height() - measure.Height()) / 2;
            childBottom = childTop + measure.Height();
            break;
        }
        case LvtkAlignment::Stretch:
            childTop = 0;
            childBottom = available.Height();
            break;
        default:
            throw std::runtime_error("Invalid vertical alignment.");
        }
        LvtkRectangle layoutRc{childLeft, childTop, childRight - childLeft, childBottom - childTop};
        if (childInfo.childType == ChildType::Popup)
        {
            layoutRc = GetAnchorRect(available, layoutRc, childInfo.anchor);
        }

        child->Layout(layoutRc);
    }
    return available;
}

LvtkRectangle LvtkRootElement::GetChildBorder(LvtkElement *element)
{
    LvtkRectangle result = element->getBorderBounds();

    while (true)
    {
        element = element->Parent();
        if (element == this)
        {
            return result;
        }
        const LvtkRectangle &bounds = element->ClientBounds();

        result = result.Translate(bounds.Left(), bounds.Top());
    }
}

LvtkRectangle LvtkRootElement::GetAnchorRect(LvtkSize available_, LvtkRectangle rc, LvtkElement *anchor)
{
    const double padding = 4;
    LvtkRectangle availableRect{padding, padding, available_.Width() - 2 * padding, available_.Height() - 2 * padding};

    LvtkRectangle anchorBorder = GetChildBorder(anchor);

    // position below.
    LvtkRectangle result{anchorBorder.Left() + 8, anchorBorder.Bottom(), rc.Width(), rc.Height()};

    if (result.Bottom() > availableRect.Bottom())
    {
        if (anchorBorder.Top() - rc.Height() > -availableRect.Top())
        {
            // position above.
            result = LvtkRectangle(result.Left(), anchorBorder.Top() - rc.Height(), result.Width(), result.Height());
        }
        else
        {
            if (availableRect.Height() > result.Height())
            {
                // position justified against bottom.
                result = LvtkRectangle(result.Left(), availableRect.Bottom() - rc.Height(), result.Width(), result.Height());
            }
            else
            {
                // position justified against top.
                result = LvtkRectangle(result.Left(), availableRect.Top(), result.Width(), result.Height());
            }
        }
    }
    if (result.Right() > availableRect.Right())
    {
        if (result.Width() < availableRect.Width())
        {
            result = LvtkRectangle(availableRect.Right() - result.Width(), result.Top(), result.Width(), result.Height());
        }
        else
        {
            result = LvtkRectangle(availableRect.Left(), result.Top(), result.Width(), result.Height());
        }
    }
    return result;
}

void LvtkRootElement::DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds)
{
    for (size_t i = 0; i < childInfos.size(); ++i)
    {
        dc.check_status();

        
        auto &childInfo = childInfos[i];
        dc.save();
        auto savedFillRule = dc.get_fill_rule();
        dc.set_fill_rule(cairo_fill_rule_t::CAIRO_FILL_RULE_EVEN_ODD);
        for (size_t j = i + 1; j < childInfos.size(); ++j)
        {
            auto &childAbove = childInfos[j];
            LvtkRectangle screenRect{Window()->Size()};

            dc.rectangle(screenRect);
            dc.rectangle(childAbove.child->ScreenBounds());
            dc.clip();
        }
        dc.set_fill_rule(savedFillRule);
        childInfo.child->Draw(dc, clipBounds);
        dc.check_status();
        dc.restore();
        dc.check_status();
    }
}
LvtkRootElement::LvtkRootElement()
{
    Style()
    .HorizontalAlignment(LvtkAlignment::Stretch)
    .VerticalAlignment(LvtkAlignment::Stretch);
}
