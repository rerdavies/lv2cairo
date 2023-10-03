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

#include "lvtk/LvtkContainerElement.hpp"

using namespace lvtk;

void LvtkContainerElement::AddChild(std::shared_ptr<LvtkElement> child)
{
    LvtkContainerElement::AddChild(child, children.size());
}
void LvtkContainerElement::AddChild(std::shared_ptr<LvtkElement> child, size_t position)
{
    if (child->parentElement != nullptr)
    {
        throw std::range_error("Child is already a childof another element.");
    }
    child->parentElement = this;
    this->children.insert(this->children.begin()+position,child);
    if (this->window != nullptr)
    {
        child->Mount(this->window);
    }
    InvalidateLayout();
}

void LvtkContainerElement::RemoveChild(size_t index)
{
    if (index >= children.size())
    {
        throw std::invalid_argument("Invalid index.");
    }
    auto t = children[index]; // keep it alive until we're done.
    children.erase(children.begin() + index);
    if (this->Window())
    {
        t->Unmount(this->Window());
    }
    t->parentElement = nullptr;
    InvalidateLayout();
}

bool LvtkContainerElement::RemoveChild(std::shared_ptr<LvtkElement> element)
{
    for (auto i = children.begin(); i != children.end(); ++i)
    {
        if ((*i).get() == element.get())
        {
            (*i)->parentElement = nullptr;
            if (element->window)
            {
                element->Unmount(element->window);
            }
            children.erase(i);
            InvalidateLayout();
            return true;
        }
    }
    return false;
}

size_t LvtkContainerElement::LayoutChildCount() const
{
    return children.size();
}
LvtkElement::ptr LvtkContainerElement::LayoutChild(size_t index)
{
    if (index >= children.size())
    {
        throw std::range_error("Invalid index.");
    }
    return children[index];
}

const LvtkElement *LvtkContainerElement::LayoutChild(size_t index) const
{
    if (index >= children.size())
    {
        throw std::range_error("Invalid index.");
    }
    return children[index].get();
}

void LvtkContainerElement::FinalizeLayout(const LvtkRectangle &layoutClipBounds,const LvtkRectangle &parentBounds,bool clippedInLayout)
{
    super::FinalizeLayout(layoutClipBounds,parentBounds,clippedInLayout);
    if (Style().Visibility() != LvtkVisibility::Collapsed)
    { 
        for (auto &child : LayoutChildren())
        {
            if (ClipChildren())
            {
                // peek at the forthcoming childBounds.
                LvtkRectangle  childBounds = child->bounds.translate(LvtkPoint(this->screenClientBounds.X(),this->screenClientBounds.Y()));

                LvtkRectangle newClipRect = layoutClipBounds.Intersect(this->screenClientBounds);

                bool clippedChild = !childBounds.Intersects(newClipRect);


                child->FinalizeLayout(newClipRect,screenClientBounds,clippedInLayout || clippedChild);

            } else {
                child->FinalizeLayout(layoutClipBounds,screenClientBounds,clippedInLayout);
                this->screenDrawBounds = this->screenDrawBounds.Union(child->screenDrawBounds);
            }
        }
    } else {
        clippedInLayout = true;
    }
}

bool LvtkContainerElement::FireScrollWheel(LvtkScrollWheelEventArgs &event)
{
    if (Style().Visibility() == LvtkVisibility::Visible)
    {

        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            for (int64_t i = children.size() - 1; i >= 0; --i)
            {
                auto child = children[i];
                if (child->screenBorderBounds.Contains(event.screenPoint))
                {
                    if (child->FireScrollWheel(event))
                        return true;
                }
            }
        }
        return super::FireScrollWheel(event);
    }
    return false;

}


bool LvtkContainerElement::FireMouseDown(LvtkMouseEventArgs &event)
{
    if (Style().Visibility() == LvtkVisibility::Visible)
    {

        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            for (int64_t i = children.size() - 1; i >= 0; --i)
            {
                auto child = children[i];
                if (child->screenBorderBounds.Contains(event.screenPoint))
                {
                    if (child->FireMouseDown(event))
                        return true;
                }
            }
        }
        return super::FireMouseDown(event);
    }
    return false;
}

bool LvtkContainerElement::FireMouseUp(LvtkMouseEventArgs &event)
{
    if (Style().Visibility() == LvtkVisibility::Visible)
    {

        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            for (int64_t i = children.size() - 1; i >= 0; --i)
            {
                auto child = children[i];
                if (child->screenBorderBounds.Contains(event.screenPoint))
                {
                    if (child->FireMouseUp(event))
                        return true;
                }
            }
        }
    }
    return super::FireMouseUp(event);
}

void LvtkContainerElement::UpdateMouseOver(LvtkPoint mousePosition)
{
    if (clippedInLayout)
    {
        return;
    }
    if (this->Style().Visibility() != LvtkVisibility::Visible)
    {
        LvtkPoint impossiblePoint{-1E15, -1E15};
        super::UpdateMouseOver(impossiblePoint);
        for (auto &child : children)
        {
            child->UpdateMouseOver(impossiblePoint);
        }
    }
    else
    {
        super::UpdateMouseOver(mousePosition);
        for (auto &child : children)
        {
            if (!child->clippedInLayout)
            {
                child->UpdateMouseOver(mousePosition);
            }
        }
    }
}

void LvtkContainerElement::Mount(LvtkWindow *window)
{
    if (this->window == window)
        return;
    this->window = window;
    for (auto &child : children)
    {
        child->Mount(window);
    }
    super::Mount(window);
}
void LvtkContainerElement::Unmount(LvtkWindow *window)
{
    if (this->window != nullptr)
    {
        std::vector<LvtkElement::ptr> children = this->children; // work with a copy to avoid invalidation of the iterator.
        for (auto &child : children)
        {
            child->Unmount(window);
        }
        super::Unmount(window);
        this->window = nullptr;
    }
}

void LvtkContainerElement::DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds)
{

    if (!clipBounds.Intersects(this->screenDrawBounds))
        return;

    if (this->Style().Visibility() != LvtkVisibility::Visible)
    {
        return;
    }

    if (screenBorderBounds.Width() <= 0 || screenBorderBounds.Height() < 0)
        return;

    LvtkRectangle clipRect = this->screenDrawBounds.Intersect(clipBounds);
    if (!clipRect.Empty())
    {
        bool clipped = WillDraw() || WillDrawOver();
        if (clipped)
        {
            dc.save();

            dc.rectangle(clipRect);
            dc.clip();
        }
        if (WillDraw())
        {
            dc.save();

            dc.translate(screenClientBounds.Left(), screenClientBounds.Top());

            OnDraw(dc);
            dc.restore();
        }
        bool clipChildren = ClipChildren();
        if (clipChildren)
        {
            LvtkRectangle clientClip = clipBounds.Intersect(this->screenClientBounds);
            if (!clientClip.Empty())
            {
                dc.save();
                {
                    dc.rectangle(clientClip);
                    dc.clip();
                    for (auto &child : children)
                    {
                        child->Draw(dc, clipRect);
                    }
                }
                dc.restore();
            }
        }
        else
        {
            for (auto &child : children)
            {
                child->Draw(dc, clipRect);
            }
        }
        if (WillDrawOver())
        {
            if (!clipRect.Empty())
            {
                dc.save();

                dc.rectangle(clipRect);
                dc.translate(screenClientBounds.Left(), screenClientBounds.Top());

                OnDrawOver(dc);
                dc.restore();
            }
        }
        if (clipped)
        {
            dc.restore();
        }
    }
}

LvtkSize LvtkContainerElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &dc)
{

    double width = 0, height = 0;
    for (auto &child : LayoutChildren())
    {
        if (child->Style().Visibility() != LvtkVisibility::Collapsed)
        {
            child->Measure(clientConstraint, clientAvailable, dc);

            LvtkSize measure = child->MeasuredSize();
            if (measure.Width() > width)
                width = measure.Width();
            if (measure.Height() > height)
                height = measure.Height();
        }
    }
    if (clientConstraint.Width() != 0)
    {
        width = clientConstraint.Width();
    }
    if (clientConstraint.Height() != 0)
    {
        height = clientConstraint.Height();
    }
    return LvtkSize(width, height);
}
LvtkSize LvtkContainerElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    LvtkRectangle marginRect{0, 0, available.Width(), available.Height()};

    LvtkRectangle borderRect = this->removeThickness(marginRect, Style().Margin());
    LvtkRectangle paddingRect = this->removeThickness(borderRect, Style().BorderWidth());
    LvtkRectangle clientRect = this->removeThickness(paddingRect, Style().Padding());

    for (auto &child : LayoutChildren())
    {
        if (child->Style().Visibility() != LvtkVisibility::Collapsed)
        {
            LvtkSize measure = child->MeasuredSize();
            child->Arrange(child->MeasuredSize(), context);
            double childLeft = -1, childRight = -1, childTop = -1, childBottom = -1;

            switch (child->Style().HorizontalAlignment())
            {
            case LvtkAlignment::Start:
                childLeft = 0;
                childRight = childLeft + measure.Width();
                break;
            case LvtkAlignment::End:
                childRight = clientRect.Width();
                childLeft = childRight - measure.Width();
                break;
            case LvtkAlignment::Center:
            {
                double extra = (clientRect.Width() - measure.Width()) / 2;
                childLeft = extra;
                childRight = childLeft + measure.Width();
                break;
            }
            case LvtkAlignment::Stretch:
                childLeft = 0;
                childRight = clientRect.Width();
            }
            switch (child->Style().VerticalAlignment())
            {
            case LvtkAlignment::Start:
                childTop = 0;
                childBottom = measure.Height();
                break;
            case LvtkAlignment::End:
                childBottom = clientRect.Bottom();
                childTop = childBottom - measure.Height();
                break;
            case LvtkAlignment::Center:
            {
                double extra = (clientRect.Height() - measure.Height()) / 2;
                childTop = extra;
                childBottom = childTop + measure.Height();
                break;
            }
            case LvtkAlignment::Stretch:
                childTop = marginRect.Left();
                childBottom = marginRect.Height();
                break;
            default:
                throw std::runtime_error("Invalid vertical alignment.");
            }
            LvtkRectangle layoutRc{childLeft, childTop, childRight - childLeft, childBottom - childTop};
            child->Layout(layoutRc);
        }
        else
        {
            LvtkRectangle invisibleLayout{-5000, -5000, 0, 0};
            child->Layout(invisibleLayout);
        }
    }
    return available;
}


void LvtkContainerElement::RemoveAllChildren()
{
    // do it the slow way to make sure we handle Mount/Unmount calls properly.
    while (this->children.size() != 0)
    {
        RemoveChild(children.size() - 1);
    }
}


void LvtkContainerElement::Children(const std::vector<LvtkElement::ptr> &children_)
{
    RemoveAllChildren();
    
    children.reserve(children.size());
    for (auto &child : children_)
    {
        AddChild(child);
    }
}

void LvtkContainerElement::OnLayoutComplete()
{
    super::OnLayoutComplete();
    for (auto&child: LayoutChildren())
    {
        child->OnLayoutComplete();
    }
}

const LvtkElement *LvtkContainerElement::Child(size_t index) const
{
    if (index >= ChildCount()) throw std::runtime_error("Index out of range.");
    return Children()[index].get();
}
LvtkElement::ptr LvtkContainerElement::Child(size_t index)
{
    if (index >= ChildCount()) throw std::runtime_error("Index out of range.");
    return Children()[index];
}

/// @brief Get the number of direct child elements.
// See LayoutChildren() for an explanation of logical and physical children.
size_t LvtkContainerElement::ChildCount() const
{
    return Children().size();
}





