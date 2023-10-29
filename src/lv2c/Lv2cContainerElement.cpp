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

#include "lv2c/Lv2cContainerElement.hpp"

using namespace lvtk;

void Lv2cContainerElement::AddChild(std::shared_ptr<Lv2cElement> child)
{
    Lv2cContainerElement::AddChild(child, children.size());
}
void Lv2cContainerElement::AddChild(std::shared_ptr<Lv2cElement> child, size_t position)
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

void Lv2cContainerElement::RemoveChild(size_t index)
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

bool Lv2cContainerElement::RemoveChild(std::shared_ptr<Lv2cElement> element)
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

size_t Lv2cContainerElement::LayoutChildCount() const
{
    return children.size();
}
Lv2cElement::ptr Lv2cContainerElement::LayoutChild(size_t index)
{
    if (index >= children.size())
    {
        throw std::range_error("Invalid index.");
    }
    return children[index];
}

const Lv2cElement *Lv2cContainerElement::LayoutChild(size_t index) const
{
    if (index >= children.size())
    {
        throw std::range_error("Invalid index.");
    }
    return children[index].get();
}

void Lv2cContainerElement::FinalizeLayout(const Lv2cRectangle &layoutClipBounds,const Lv2cRectangle &parentBounds,bool clippedInLayout)
{
    super::FinalizeLayout(layoutClipBounds,parentBounds,clippedInLayout);
    if (Style().Visibility() != Lv2cVisibility::Collapsed)
    { 
        for (auto &child : LayoutChildren())
        {
            if (ClipChildren())
            {
                // peek at the forthcoming childBounds.
                Lv2cRectangle  childBounds = child->bounds.translate(Lv2cPoint(this->screenClientBounds.X(),this->screenClientBounds.Y()));

                Lv2cRectangle newClipRect = layoutClipBounds.Intersect(this->screenClientBounds);

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

bool Lv2cContainerElement::FireScrollWheel(Lv2cScrollWheelEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
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


bool Lv2cContainerElement::FireMouseDown(Lv2cMouseEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
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

bool Lv2cContainerElement::FireMouseUp(Lv2cMouseEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
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

void Lv2cContainerElement::UpdateMouseOver(Lv2cPoint mousePosition)
{
    if (clippedInLayout)
    {
        return;
    }
    if (this->Style().Visibility() != Lv2cVisibility::Visible)
    {
        Lv2cPoint impossiblePoint{-1E15, -1E15};
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

void Lv2cContainerElement::Mount(Lv2cWindow *window)
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
void Lv2cContainerElement::Unmount(Lv2cWindow *window)
{
    if (this->window != nullptr)
    {
        std::vector<Lv2cElement::ptr> children = this->children; // work with a copy to avoid invalidation of the iterator.
        for (auto &child : children)
        {
            child->Unmount(window);
        }
        super::Unmount(window);
        this->window = nullptr;
    }
}

void Lv2cContainerElement::DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{

    if (!clipBounds.Intersects(this->screenDrawBounds))
        return;

    if (this->Style().Visibility() != Lv2cVisibility::Visible)
    {
        return;
    }

    if (screenBorderBounds.Width() <= 0 || screenBorderBounds.Height() < 0)
        return;

    Lv2cRectangle clipRect = this->screenDrawBounds.Intersect(clipBounds);
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
            Lv2cRectangle clientClip = clipBounds.Intersect(this->screenClientBounds);
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

Lv2cSize Lv2cContainerElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &dc)
{

    double width = 0, height = 0;
    for (auto &child : LayoutChildren())
    {
        if (child->Style().Visibility() != Lv2cVisibility::Collapsed)
        {
            child->Measure(clientConstraint, clientAvailable, dc);

            Lv2cSize measure = child->MeasuredSize();
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
    return Lv2cSize(width, height);
}
Lv2cSize Lv2cContainerElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    Lv2cRectangle marginRect{0, 0, available.Width(), available.Height()};

    Lv2cRectangle borderRect = this->removeThickness(marginRect, Style().Margin());
    Lv2cRectangle paddingRect = this->removeThickness(borderRect, Style().BorderWidth());
    Lv2cRectangle clientRect = this->removeThickness(paddingRect, Style().Padding());

    for (auto &child : LayoutChildren())
    {
        if (child->Style().Visibility() != Lv2cVisibility::Collapsed)
        {
            Lv2cSize measure = child->MeasuredSize();
            child->Arrange(child->MeasuredSize(), context);
            double childLeft = -1, childRight = -1, childTop = -1, childBottom = -1;

            switch (child->Style().HorizontalAlignment())
            {
            case Lv2cAlignment::Start:
                childLeft = 0;
                childRight = childLeft + measure.Width();
                break;
            case Lv2cAlignment::End:
                childRight = clientRect.Width();
                childLeft = childRight - measure.Width();
                break;
            case Lv2cAlignment::Center:
            {
                double extra = (clientRect.Width() - measure.Width()) / 2;
                childLeft = extra;
                childRight = childLeft + measure.Width();
                break;
            }
            case Lv2cAlignment::Stretch:
                childLeft = 0;
                childRight = clientRect.Width();
            }
            switch (child->Style().VerticalAlignment())
            {
            case Lv2cAlignment::Start:
                childTop = 0;
                childBottom = measure.Height();
                break;
            case Lv2cAlignment::End:
                childBottom = clientRect.Bottom();
                childTop = childBottom - measure.Height();
                break;
            case Lv2cAlignment::Center:
            {
                double extra = (clientRect.Height() - measure.Height()) / 2;
                childTop = extra;
                childBottom = childTop + measure.Height();
                break;
            }
            case Lv2cAlignment::Stretch:
                childTop = marginRect.Left();
                childBottom = marginRect.Height();
                break;
            default:
                throw std::runtime_error("Invalid vertical alignment.");
            }
            Lv2cRectangle layoutRc{childLeft, childTop, childRight - childLeft, childBottom - childTop};
            child->Layout(layoutRc);
        }
        else
        {
            Lv2cRectangle invisibleLayout{-5000, -5000, 0, 0};
            child->Layout(invisibleLayout);
        }
    }
    return available;
}


void Lv2cContainerElement::RemoveAllChildren()
{
    // do it the slow way to make sure we handle Mount/Unmount calls properly.
    while (this->children.size() != 0)
    {
        RemoveChild(children.size() - 1);
    }
}


void Lv2cContainerElement::Children(const std::vector<Lv2cElement::ptr> &children_)
{
    RemoveAllChildren();
    
    children.reserve(children.size());
    for (auto &child : children_)
    {
        AddChild(child);
    }
}

void Lv2cContainerElement::OnLayoutComplete()
{
    super::OnLayoutComplete();
    for (auto&child: LayoutChildren())
    {
        child->OnLayoutComplete();
    }
}

const Lv2cElement *Lv2cContainerElement::Child(size_t index) const
{
    if (index >= ChildCount()) throw std::runtime_error("Index out of range.");
    return Children()[index].get();
}
Lv2cElement::ptr Lv2cContainerElement::Child(size_t index)
{
    if (index >= ChildCount()) throw std::runtime_error("Index out of range.");
    return Children()[index];
}

/// @brief Get the number of direct child elements.
// See LayoutChildren() for an explanation of logical and physical children.
size_t Lv2cContainerElement::ChildCount() const
{
    return Children().size();
}





