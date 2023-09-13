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

#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkLog.hpp"

using namespace lvtk;

// StyleContext LvtkVerticalStackElement::GetHorizontalMeasureContext(double width, LvtkElement *element)
// {
//     StyleContext result;
//     if (element->Style().HorizontalAlignment() == LvtkAlignment::Stretch)
//     {
//         result = {width};
//     }
//     else
//     {
//         result = {width};
//     }
//     return result;
// }

LvtkSize LvtkVerticalStackElement::MeasureClient(LvtkSize constraint, LvtkSize available, LvtkDrawingContext &context)
{
    this->clipChildren = false;
    childInfos.resize(ChildCount());

    double height = 0;
    double maxWidth = 0;
    double autoCount = 0;

    for (size_t i = 0; i < this->ChildCount(); ++i)
    {
        auto &child = this->Children()[i];
        auto &childInfo = this->childInfos[i];

        if (child->Style().Visibility() == LvtkVisibility::Collapsed)
            continue;

        if (child->Style().VerticalAlignment() == LvtkAlignment::Stretch && constraint.Height() != 0) // only stretch children if we have a fixed width!
        {
            autoCount += 1;
        }
        else
        {
            LvtkSize childConstraint{0, 0};
            if (child->Style().HorizontalAlignment() == LvtkAlignment::Stretch)
            {
                childConstraint.Width(constraint.Width());
            }
            child->Measure(constraint, available, context);
            LvtkSize measure = child->MeasuredSize();
            height += measure.Height();
            if (measure.Width() > maxWidth)
            {
                maxWidth = measure.Width();
            }
            childInfo.measureSize = measure;
            childInfo.arrangeSize = LvtkSize(0, measure.Height());
        }
    }

    if (autoCount != 0)
    {

        double extra = (available.Height() - height) / autoCount;
        if (extra < 0)
            extra = 0;
        LvtkSize childAvailable = LvtkSize(available.Width(), extra);

        for (size_t i = 0; i < ChildCount(); ++i)
        {
            auto &child = Children()[i];
            auto &childInfo = childInfos[i];

            if (child->Style().Visibility() == LvtkVisibility::Collapsed)
                continue;

            if (child->Style().VerticalAlignment() == LvtkAlignment::Stretch)
            {

                LvtkSize measureSize{constraint.Width(), extra};

                child->Measure(measureSize, childAvailable, context);
                LvtkSize measure = child->MeasuredSize();
                childInfo.measureSize = measure;
                childInfo.arrangeSize = LvtkSize(0, extra);
                height += measure.Height();
                if (measure.Width() > maxWidth)
                {
                    maxWidth = measure.Width();
                }
            }
        }
    }
    for (size_t i = 0; i < this->ChildCount(); ++i)
    {
        auto &child = this->Children()[i];
        if (child->Style().HorizontalAlignment() == LvtkAlignment::Stretch)
        {
            auto &childInfo = this->childInfos[i];
            childInfo.measureSize.Width(maxWidth);
        }
    }

    for (ChildInfo &childInfo : childInfos)
    {
        childInfo.arrangeSize.Width(maxWidth);
    }

    LvtkSize result = LvtkSize(maxWidth, height);

    if (result.Width() < 0)
        result.Width(0);
    if (result.Height() < 0)
        result.Height(0);

    if (result.Width() > available.Width())
    {
        result.Width(available.Width());
    }
    if (result.Height() > available.Height())
    {
        clipChildren = true;
        result.Height(available.Height());
    }
    if (Style().Width().PixelValue() != 0)
    {
        result.Width(Style().Width().PixelValue());
    }
    else if (this->Style().HorizontalAlignment() == LvtkAlignment::Stretch)
    {
        result.Width(available.Width());
    }

    if (Style().Height().PixelValue() != 0)
    {
        result.Height(Style().Height().PixelValue());
    }
    else if (this->Style().VerticalAlignment() == LvtkAlignment::Stretch)
    {
        result.Height(available.Height());
    }
    if (result.Width() > maxWidth)
    {
        clipChildren = true;
    }
    if (result.Height() > height)
    {
        clipChildren = true;
    }

    return result;
}

LvtkSize LvtkVerticalStackElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{

    LvtkRectangle arrangeRectangle = LvtkRectangle(0, 0, available.Width(), available.Height());

    LvtkRectangle borderRectangle = removeThickness(arrangeRectangle, Style().Margin());
    LvtkRectangle borderInsideRectangle = removeThickness(borderRectangle, Style().BorderWidth());
    LvtkRectangle clientRect = removeThickness(borderInsideRectangle, Style().Padding());

    float left = 0;
    float right = clientRect.Width();
    if (right < 0)
        right = 0;
    float top = 0;

    LvtkSize clientSize{clientRect.Width(), clientRect.Height()};

    for (size_t i = 0; i < ChildCount(); ++i)
    {
        auto &child = Children()[i];
        auto &childInfo = childInfos[i];

        if (child->Style().Visibility() == LvtkVisibility::Collapsed)
        {
            child->Layout({0, 0, 0, 0});
            continue;
        }
        double childLeft = -1, childTop = -1, childRight = -1, childBottom = -1;

        switch (child->Style().HorizontalAlignment())
        {
        case LvtkAlignment::Start:
            childLeft = left;
            childRight = childLeft + childInfo.measureSize.Width();
            break;
        case LvtkAlignment::End:
            childRight = right;
            childLeft = childRight - childInfo.measureSize.Width();
            break;
        case LvtkAlignment::Center:
        {
            childLeft = left + (clientRect.Width() - childInfo.measureSize.Width()) * 0.5;
            childRight = childLeft + childInfo.measureSize.Width();
            break;
        }
        case LvtkAlignment::Stretch:
            childLeft = left;
            childRight = right;
            break;
        }
        switch (child->Style().VerticalAlignment())
        {
        case LvtkAlignment::Start:
            childTop = top;
            childBottom = top + childInfo.measureSize.Height();
            break;
        case LvtkAlignment::End:
            childBottom = top + childInfo.measureSize.Height();
            childTop = childBottom - childInfo.measureSize.Height();
            break;
        case LvtkAlignment::Center:
        {
            double extra = (childInfo.arrangeSize.Height() - childInfo.measureSize.Height()) * 0.5;
            childTop = top + extra;
            childBottom = childTop + childInfo.measureSize.Height();
            break;
        }
        case LvtkAlignment::Stretch:
            childTop = top;
            childBottom = childTop + childInfo.arrangeSize.Height();
            break;
        }
        top += childInfo.arrangeSize.Height();
        LvtkRectangle finalPlacement{childLeft, childTop, childRight - childLeft, childBottom - childTop};
        child->Arrange(LvtkSize(finalPlacement.Width(), finalPlacement.Height()), context);
        child->Layout(finalPlacement);
    }
    return available;
}

LvtkVerticalStackElement::LvtkVerticalStackElement()
{
    Style().HorizontalAlignment(LvtkAlignment::Stretch);
}
LvtkVerticalStackElement::~LvtkVerticalStackElement()
{
}

void LvtkVerticalStackElement::Measure(LvtkSize constraint, LvtkSize available, LvtkDrawingContext &context)
{
    // us MeasureClient

    LvtkElement::Measure(constraint, available, context);
}

bool LvtkVerticalStackElement::ClipChildren() const
{
    return clipChildren;
}
