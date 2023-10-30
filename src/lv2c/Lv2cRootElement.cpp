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

#include "lv2c/Lv2cRootElement.hpp"
#include "lv2c/Lv2cWindow.hpp"

#define XK_MISCELLANY
#include <X11/keysymdef.h>

using namespace lv2c;

void Lv2cRootElement::AddChildEx(
    ChildType childType,
    Lv2cElement::ptr child,
    double zOrder,
    Lv2cElement *anchor,
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

void Lv2cRootElement::AddChild(std::shared_ptr<Lv2cElement> child)
{
    AddChildEx(ChildType::Normal, child, 0, nullptr, []() {});
}
bool Lv2cRootElement::RemoveChild(std::shared_ptr<Lv2cElement> element)
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
void Lv2cRootElement::RemoveChild(size_t index)
{
    Window()->Invalidate(childInfos[index].child->ScreenBounds());
    childInfos[index].onClose();
    childInfos.erase(childInfos.begin() + index);
    super::RemoveChild(index);
}

void Lv2cRootElement::UpdateMouseOver(Lv2cPoint mousePosition)
{
    Lv2cPoint currentMousePosition = mousePosition;
    Lv2cPoint noPoint{-10000, -1000};
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

bool Lv2cRootElement::FireMouseDown(Lv2cMouseEventArgs &event)
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

bool Lv2cRootElement::FireKeyDown(const Lv2cKeyboardEventArgs &event)
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

bool Lv2cRootElement::HandlePopupKeys(Lv2cElement::ptr child, const Lv2cKeyboardEventArgs &event)
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

void Lv2cRootElement::AddPopup(std::shared_ptr<Lv2cElement> child, Lv2cElement *anchor, std::function<void(void)> &&onClose)
{
    AddChildEx(ChildType::Popup, child, 100.0, anchor, std::move(onClose));
}

Lv2cSize Lv2cRootElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    for (auto &childInfo : childInfos)
    {

        auto child = childInfo.child;

        Lv2cSize measure = child->MeasuredSize();
        child->Arrange(child->MeasuredSize(), context);
        double childLeft = -1, childRight = -1, childTop = -1, childBottom = -1;

        switch (child->Style().HorizontalAlignment())
        {
        case Lv2cAlignment::Start:
            childLeft = 0;
            childRight = measure.Width();
            break;
        case Lv2cAlignment::End:
            childRight = available.Width();
            childLeft = childRight - measure.Width();
            break;
        case Lv2cAlignment::Center:
        {
            double extra = (available.Width() - measure.Width()) / 2;
            childLeft = extra;
            childRight = measure.Width();
            break;
        }
        case Lv2cAlignment::Stretch:
            childLeft = 0;
            childRight = available.Width();
        }
        switch (child->Style().VerticalAlignment())
        {
        case Lv2cAlignment::Start:
            childTop = 0;
            childBottom = measure.Height();
            break;
        case Lv2cAlignment::End:
            childBottom = available.Height();
            childTop = childBottom - measure.Height();
            break;
        case Lv2cAlignment::Center:
        {
            childTop = (available.Height() - measure.Height()) / 2;
            childBottom = childTop + measure.Height();
            break;
        }
        case Lv2cAlignment::Stretch:
            childTop = 0;
            childBottom = available.Height();
            break;
        default:
            throw std::runtime_error("Invalid vertical alignment.");
        }
        Lv2cRectangle layoutRc{childLeft, childTop, childRight - childLeft, childBottom - childTop};
        if (childInfo.childType == ChildType::Popup)
        {
            layoutRc = GetAnchorRect(available, layoutRc, childInfo.anchor);
        }

        child->Layout(layoutRc);
    }
    return available;
}

Lv2cRectangle Lv2cRootElement::GetChildBorder(Lv2cElement *element)
{
    Lv2cRectangle result = element->getBorderBounds();

    while (true)
    {
        element = element->Parent();
        if (element == this)
        {
            return result;
        }
        const Lv2cRectangle &bounds = element->ClientBounds();

        result = result.Translate(bounds.Left(), bounds.Top());
    }
}

Lv2cRectangle Lv2cRootElement::GetAnchorRect(Lv2cSize available_, Lv2cRectangle rc, Lv2cElement *anchor)
{
    const double padding = 4;
    Lv2cRectangle availableRect{padding, padding, available_.Width() - 2 * padding, available_.Height() - 2 * padding};

    Lv2cRectangle anchorBorder = GetChildBorder(anchor);

    // position below.
    Lv2cRectangle result{anchorBorder.Left() + 8, anchorBorder.Bottom(), rc.Width(), rc.Height()};

    if (result.Bottom() > availableRect.Bottom())
    {
        if (anchorBorder.Top() - rc.Height() > -availableRect.Top())
        {
            // position above.
            result = Lv2cRectangle(result.Left(), anchorBorder.Top() - rc.Height(), result.Width(), result.Height());
        }
        else
        {
            if (availableRect.Height() > result.Height())
            {
                // position justified against bottom.
                result = Lv2cRectangle(result.Left(), availableRect.Bottom() - rc.Height(), result.Width(), result.Height());
            }
            else
            {
                // position justified against top.
                result = Lv2cRectangle(result.Left(), availableRect.Top(), result.Width(), result.Height());
            }
        }
    }
    if (result.Right() > availableRect.Right())
    {
        if (result.Width() < availableRect.Width())
        {
            result = Lv2cRectangle(availableRect.Right() - result.Width(), result.Top(), result.Width(), result.Height());
        }
        else
        {
            result = Lv2cRectangle(availableRect.Left(), result.Top(), result.Width(), result.Height());
        }
    }
    return result;
}

void Lv2cRootElement::DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
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
            Lv2cRectangle screenRect{Window()->Size()};

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
Lv2cRootElement::Lv2cRootElement()
{
    Style()
    .HorizontalAlignment(Lv2cAlignment::Stretch)
    .VerticalAlignment(Lv2cAlignment::Stretch);
}
