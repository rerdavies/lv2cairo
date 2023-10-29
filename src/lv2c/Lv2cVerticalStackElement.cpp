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

#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cLog.hpp"

using namespace lvtk;

// StyleContext Lv2cVerticalStackElement::GetHorizontalMeasureContext(double width, Lv2cElement *element)
// {
//     StyleContext result;
//     if (element->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
//     {
//         result = {width};
//     }
//     else
//     {
//         result = {width};
//     }
//     return result;
// }

Lv2cSize Lv2cVerticalStackElement::MeasureClient(Lv2cSize constraint, Lv2cSize available, Lv2cDrawingContext &context)
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

        if (child->Style().Visibility() == Lv2cVisibility::Collapsed)
            continue;

        if (child->Style().VerticalAlignment() == Lv2cAlignment::Stretch && constraint.Height() != 0) // only stretch children if we have a fixed width!
        {
            autoCount += 1;
        }
        else
        {
            Lv2cSize childConstraint{0, 0};
            if (child->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
            {
                childConstraint.Width(constraint.Width());
            }
            child->Measure(constraint, available, context);
            Lv2cSize measure = child->MeasuredSize();
            height += measure.Height();
            if (measure.Width() > maxWidth)
            {
                maxWidth = measure.Width();
            }
            childInfo.measureSize = measure;
            childInfo.arrangeSize = Lv2cSize(0, measure.Height());
        }
    }

    if (autoCount != 0)
    {

        double extra = (available.Height() - height) / autoCount;
        if (extra < 0)
            extra = 0;
        Lv2cSize childAvailable = Lv2cSize(available.Width(), extra);

        for (size_t i = 0; i < ChildCount(); ++i)
        {
            auto &child = Children()[i];
            auto &childInfo = childInfos[i];

            if (child->Style().Visibility() == Lv2cVisibility::Collapsed)
                continue;

            if (child->Style().VerticalAlignment() == Lv2cAlignment::Stretch)
            {

                Lv2cSize measureSize{constraint.Width(), extra};

                child->Measure(measureSize, childAvailable, context);
                Lv2cSize measure = child->MeasuredSize();
                childInfo.measureSize = measure;
                childInfo.arrangeSize = Lv2cSize(0, extra);
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
        if (child->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
        {
            auto &childInfo = this->childInfos[i];
            childInfo.measureSize.Width(maxWidth);
        }
    }

    for (ChildInfo &childInfo : childInfos)
    {
        childInfo.arrangeSize.Width(maxWidth);
    }

    Lv2cSize result = Lv2cSize(maxWidth, height);

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
    else if (this->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
    {
        result.Width(available.Width());
    }

    if (Style().Height().PixelValue() != 0)
    {
        result.Height(Style().Height().PixelValue());
    }
    else if (this->Style().VerticalAlignment() == Lv2cAlignment::Stretch)
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

Lv2cSize Lv2cVerticalStackElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{

    Lv2cRectangle arrangeRectangle = Lv2cRectangle(0, 0, available.Width(), available.Height());

    Lv2cRectangle borderRectangle = removeThickness(arrangeRectangle, Style().Margin());
    Lv2cRectangle borderInsideRectangle = removeThickness(borderRectangle, Style().BorderWidth());
    Lv2cRectangle clientRect = removeThickness(borderInsideRectangle, Style().Padding());

    float left = 0;
    float right = clientRect.Width();
    if (right < 0)
        right = 0;
    float top = 0;

    Lv2cSize clientSize{clientRect.Width(), clientRect.Height()};

    for (size_t i = 0; i < ChildCount(); ++i)
    {
        auto &child = Children()[i];
        auto &childInfo = childInfos[i];

        if (child->Style().Visibility() == Lv2cVisibility::Collapsed)
        {
            child->Layout({0, 0, 0, 0});
            continue;
        }
        double childLeft = -1, childTop = -1, childRight = -1, childBottom = -1;

        switch (child->Style().HorizontalAlignment())
        {
        case Lv2cAlignment::Start:
            childLeft = left;
            childRight = childLeft + childInfo.measureSize.Width();
            break;
        case Lv2cAlignment::End:
            childRight = right;
            childLeft = childRight - childInfo.measureSize.Width();
            break;
        case Lv2cAlignment::Center:
        {
            childLeft = left + (clientRect.Width() - childInfo.measureSize.Width()) * 0.5;
            childRight = childLeft + childInfo.measureSize.Width();
            break;
        }
        case Lv2cAlignment::Stretch:
            childLeft = left;
            childRight = right;
            break;
        }
        switch (child->Style().VerticalAlignment())
        {
        case Lv2cAlignment::Start:
            childTop = top;
            childBottom = top + childInfo.measureSize.Height();
            break;
        case Lv2cAlignment::End:
            childBottom = top + childInfo.measureSize.Height();
            childTop = childBottom - childInfo.measureSize.Height();
            break;
        case Lv2cAlignment::Center:
        {
            double extra = (childInfo.arrangeSize.Height() - childInfo.measureSize.Height()) * 0.5;
            childTop = top + extra;
            childBottom = childTop + childInfo.measureSize.Height();
            break;
        }
        case Lv2cAlignment::Stretch:
            childTop = top;
            childBottom = childTop + childInfo.arrangeSize.Height();
            break;
        }
        top += childInfo.arrangeSize.Height();
        Lv2cRectangle finalPlacement{childLeft, childTop, childRight - childLeft, childBottom - childTop};
        child->Arrange(Lv2cSize(finalPlacement.Width(), finalPlacement.Height()), context);
        child->Layout(finalPlacement);
    }
    return available;
}

Lv2cVerticalStackElement::Lv2cVerticalStackElement()
{
    Style().HorizontalAlignment(Lv2cAlignment::Stretch);
}
Lv2cVerticalStackElement::~Lv2cVerticalStackElement()
{
}

void Lv2cVerticalStackElement::Measure(Lv2cSize constraint, Lv2cSize available, Lv2cDrawingContext &context)
{
    // us MeasureClient

    Lv2cElement::Measure(constraint, available, context);
}

bool Lv2cVerticalStackElement::ClipChildren() const
{
    return clipChildren;
}
