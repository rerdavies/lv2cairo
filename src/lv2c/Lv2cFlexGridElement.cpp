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

#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cLog.hpp"

using namespace lv2c;

Lv2cSize Lv2cFlexGridElement::MeasureClient(Lv2cSize constraint, Lv2cSize available, Lv2cDrawingContext &context)
{
    this->clipChildren = false;
    flexRowGap = Style().RowGap().PixelValue();
    flexColumnGap = Style().ColumnGap().PixelValue();

    childInfos.resize(0);
    flows.resize(0);
    invisibleChildren.resize(0);

    bool canWrap = style.FlexWrap() == Lv2cFlexWrap::Wrap;

    size_t nFlow = 0;
    this->flows.push_back(FlowInfo());
    this->childInfos.reserve(this->Children().size());
    double maxWidth = available.Width();
    double maxHeight = available.Height();

    bool unconstrainedHeight = false;

    if (style.FlexDirection() == Lv2cFlexDirection::Row)
    {
        // pass 1: measure all fixed elements, and perform row breaking.
        for (auto element:  this->Children())
        {
            FlowInfo&currentFlow = flows[nFlow];

            auto gapSize = currentFlow.childStart == currentFlow.childEnd ? 0: this->flexColumnGap;

            if (element->Style().Visibility() == Lv2cVisibility::Collapsed)
            {
                invisibleChildren.push_back(element.get());
                continue;
            }
            size_t childPosition = childInfos.size();

            if (element->Style().HorizontalAlignment() == Lv2cAlignment::Stretch && constraint.Width() != 0) // only stretch children if we are stretchy.
            {
                ChildInfo childInfo;
                childInfo.child = element.get();
                childInfo.nFlow = nFlow;
                childInfo.wantsExtra = true;
                currentFlow.extraCount += 1;
                childInfos.push_back(childInfo);
                if (element->Style().VerticalAlignment() == Lv2cAlignment::Stretch)
                {
                    currentFlow.childHasStretch = true;
                }
                int childPosition = childInfos.size()-1;

                if (canWrap)
                {
                    // in the current row, but start a new row for the next item.
                    ++nFlow; 
                    currentFlow.fixedSpace += gapSize; // take out the gap now so we don't have to deal with it later.
                    currentFlow.childEnd = childPosition+1;
                    FlowInfo newFlow = FlowInfo();
                    newFlow.childStart = childPosition+1;
                    newFlow.childEnd = childPosition+1;
                    flows.push_back(newFlow);
                } else {
                    currentFlow.fixedSpace += gapSize; // take out the gap now so we don't have to deal with it later.
                    currentFlow.childEnd = childPosition+1;
                }
            } else 
            {
                element->Measure(Lv2cSize(0,0),available,context);
                ChildInfo info;
                info.nFlow = nFlow;
                info.measuredSize = element->MeasuredSize();
                info.hasMeasure = true;
                double rowSize = currentFlow.fixedSpace + info.measuredSize.Width();
                FlowInfo *pCurrentFlow = &(currentFlow);
                if (canWrap && rowSize >= maxWidth && currentFlow.childStart == currentFlow.childEnd) // one element on row, that's too wide.
                {
                    // first element in a row, but it's wider than maxWidth.
                    // put it in the current row, and start a new one.
                    currentFlow.childEnd = childPosition+1;
                    currentFlow.fixedSpace = maxWidth;
                    FlowInfo newInfo;
                    newInfo.childStart = childPosition+1;
                    newInfo.childEnd = childPosition+1;
                    info.measuredSize.Width(maxWidth);
                    flows.push_back(newInfo);
                    pCurrentFlow = &(flows[nFlow]); // moved after the push!
                    ++nFlow;
                    // and put this element into the current flow.
                } else if (canWrap && rowSize+gapSize >= maxWidth)
                {
                    // row is full. Put it in the next row.
                    gapSize = 0;
                    ++nFlow;
                    // if it fills the next row, reduce the width.
                    if (info.measuredSize.Width() > maxWidth)
                    {
                        info.measuredSize.Width(maxWidth);
                    }
                    info.nFlow = nFlow;
                    currentFlow.childEnd = childPosition;
                    FlowInfo newInfo;
                    newInfo.childStart = childPosition;
                    newInfo.childEnd = childPosition+1;
                    newInfo.fixedSpace = info.measuredSize.Width();
                    flows.push_back(newInfo);
                    pCurrentFlow = &flows[nFlow];
                } else {
                    currentFlow.childEnd = childPosition+1;
                    pCurrentFlow->fixedSpace += info.measuredSize.Width() + gapSize;
                }
                if (element->Style().VerticalAlignment() != Lv2cAlignment::Stretch || constraint.Width() == 0)
                {
                    double rowHeight = info.measuredSize.Height();
                    if (rowHeight > pCurrentFlow->flowSecondAxisSize)
                    {
                        pCurrentFlow->flowSecondAxisSize = rowHeight;
                    }
                } else {
                    pCurrentFlow->childHasStretch = true;
                }
                
                info.child = element.get();
                childInfos.push_back(info);
            }
        }
        // pass 1a: Determine size of elements that are autowidth, but not autoheight.
        for (auto & flow: flows)
        {
            if (!flow.childHasStretch) {
                double extraCount = 0;
                double fixedWidth = flow.fixedSpace;
                for (size_t i = flow.childStart; i < flow.childEnd; ++i)
                {
                    auto & child = childInfos[i].child;

                    if (child->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
                    {
                        extraCount += 1;
                    } 
                }
                if (extraCount != 0)
                {
                    double extra = (available.Width()-fixedWidth)/extraCount;
                    for (size_t i = flow.childStart; i < flow.childEnd; ++i)
                    {
                        auto & child = childInfos[i].child;

                        if (child->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
                        {
                            ChildInfo &childInfo = childInfos[i];
                            child->Measure(Lv2cSize(extra,0),Lv2cSize(extra,available.Height()),context);
                            childInfo.measuredSize = child->MeasuredSize();

                            childInfo.measuredSize.Width(extra);

                            childInfo.hasMeasure = true;
                            childInfo.wantsExtra = false;
                            flow.fixedSpace += extra;
                            if (childInfo.measuredSize.Height() > flow.flowSecondAxisSize)
                            {
                                flow.flowSecondAxisSize = childInfo.measuredSize.Height();
                            }
                        }
                    }
                }
            }
        }

        // pass 2: Determine the line heights for all childHasStretch lines.
        double fixedLineHeight = 0;
        int autoLines = 0;
        for (auto & flow: flows)
        {
            if (flow.childHasStretch) {
                ++autoLines;
            } else {
                fixedLineHeight += flow.flowSecondAxisSize;
            }
        }
        if (flows.size() != 0)
        {
            fixedLineHeight += this->flexRowGap*(flows.size()-1);
        }
        double lineExtra = 0;
        if (autoLines != 0)
        {
            lineExtra = (available.Height()-fixedLineHeight)/autoLines;
            for (auto &flow: flows)
            {
                if (flow.childHasStretch)
                {
                    flow.flowSecondAxisSize = lineExtra;
                }
            }
        }
        // pass 3: do measures of all children with both auto width and auto height
        for (size_t i = 0; i < childInfos.size(); ++i)
        {
            ChildInfo &childInfo = childInfos[i];
            FlowInfo &flowInfo = this->flows[childInfo.nFlow];
            if (childInfo.wantsExtra)
            {
                double childExtra = (maxWidth-flowInfo.fixedSpace)/flowInfo.extraCount;
                if (childExtra < 0) childExtra = 0;
                flowInfo.extraSpace = childExtra;
                auto child = childInfos[i].child;

                child->Measure(Lv2cSize(childExtra,flowInfo.flowSecondAxisSize), Lv2cSize(childExtra,flowInfo.flowSecondAxisSize),context);
                auto measure = child->MeasuredSize();
                if (measure.Width() > childExtra)
                {
                    measure.Width(childExtra);
                }
                if (measure.Height() > flowInfo.flowSecondAxisSize)
                {
                    measure.Height(flowInfo.flowSecondAxisSize);
                }
                childInfo.measuredSize = measure;
            }
        }

        // calculate the measure.
        double width= 0, height=0;
        for (size_t nFlow = 0; nFlow < flows.size(); ++nFlow)
        {
            FlowInfo &flow = flows[nFlow];
            height += flow.flowSecondAxisSize;
            if (flow.fixedSpace > width)
            {
                width = flow.fixedSpace;
            }
        }
        if (flows.size() != 0)
        {
            height += this->flexRowGap*flows.size();
        }
        if (width > available.Width()) 
        {
            width = available.Width();
            this->clipChildren = true;
        }
        if (height > available.Height())
        {
            height = available.Height();
            this->clipChildren = true;
        }
        Lv2cSize result { width,height};
        return result;
    } else {
        // column flow.
        // xxx
        // pass 1: measure all fixed elements, and perform column breaking.
        for (auto&element: Children())
        {
            FlowInfo&currentFlow = flows[nFlow];

            if (element->Style().Visibility() == Lv2cVisibility::Collapsed)
            {
                invisibleChildren.push_back(element.get());
                continue;
            }
            size_t childPosition = childInfos.size();

            double rowGap = currentFlow.childStart == currentFlow.childEnd ? 0: this->flexRowGap;

            if (element->Style().HorizontalAlignment() == Lv2cAlignment::Stretch && constraint.Width() != 0) // only stretch if we are stretchy.
            {
                ChildInfo childInfo;
                childInfo.nFlow = nFlow;
                childInfo.wantsExtra = true;
                currentFlow.extraCount += 1;
                childInfo.child = element.get();
                childInfos.push_back(childInfo);
                if (element->Style().HorizontalAlignment() == Lv2cAlignment::Stretch)
                {
                    currentFlow.childHasStretch = true;
                }

                if (canWrap)
                {
                    ++nFlow;
                    currentFlow.childEnd = childPosition+1;
                    FlowInfo newFlow = FlowInfo();
                    newFlow.childStart = childPosition+1;
                    newFlow.childEnd = childPosition+1;

                    flows.push_back(newFlow);
                } else {
                    currentFlow.childEnd = childPosition+1;
                    // add the extra now.
                    currentFlow.flowSecondAxisSize += rowGap;
                }
            } else {
                element->Measure(Lv2cSize(0,0),available,context);
                ChildInfo info;
                info.nFlow = nFlow;
                info.measuredSize = element->MeasuredSize();
                double columnSize = currentFlow.fixedSpace + info.measuredSize.Height();
                FlowInfo *pCurrentFlow = &(currentFlow);
                if (canWrap && columnSize >= maxHeight && currentFlow.childStart == currentFlow.childEnd) // one element on row, that's too wide.
                {
                    // first element in a row, but it's taller than maxHeight
                    // put it in the current column, and start a new one.
                    currentFlow.childEnd = childPosition+1;
                    currentFlow.fixedSpace = maxHeight;
                    FlowInfo newInfo;
                    newInfo.childStart = childPosition+1;
                    newInfo.childEnd = childPosition+1;
                    rowGap = 0;
                    info.measuredSize.Height(maxHeight);
                    flows.push_back(newInfo);
                    pCurrentFlow = &(flows[nFlow]);
                    ++nFlow;
                    rowGap = 0;
                    // and put this element into the current flow.
                } else if (canWrap && columnSize + rowGap >= maxHeight)
                {
                    // row is full. Put it in the next row.
                    ++nFlow;
                    info.nFlow = nFlow;
                    // if it fills the next row, reduce the width.
                    if (info.measuredSize.Height() > maxHeight)
                    {
                        info.measuredSize.Height(maxHeight);
                    }
                    currentFlow.childEnd = childPosition;
                    FlowInfo newInfo;
                    newInfo.childStart = childPosition;
                    newInfo.childEnd = childPosition+1;
                    newInfo.fixedSpace = info.measuredSize.Height();
                    flows.push_back(newInfo);
                    pCurrentFlow = &flows[nFlow];
                } else {
                    currentFlow.childEnd = childPosition+1;
                    pCurrentFlow->fixedSpace += info.measuredSize.Height() + rowGap;
                }
                double rowWidth = info.measuredSize.Width();
                if (rowWidth > pCurrentFlow->flowSecondAxisSize)
                {
                    pCurrentFlow->flowSecondAxisSize = rowWidth;
                }
                info.child = element.get();
                childInfos.push_back(info);
            }
        }

        // pass 1a: all horizontal stretch items take maximum width of their column.

        for (size_t i = 0; i < childInfos.size(); ++i)
        {
            auto& child = childInfos[i].child;
            if (child->Style().HorizontalAlignment() == Lv2cAlignment::Stretch
            && child->Style().VerticalAlignment() != Lv2cAlignment::Stretch)
            {
                ChildInfo &info = childInfos[i];
                auto&flow = flows[info.nFlow];
                info.hasMeasure = true;
                info.measuredSize.Width(flow.flowSecondAxisSize);
            }
        }
        // pass 2: measure all horizontal stretch elements in fixed-width columns.
        for (size_t i = 0; i < childInfos.size(); ++i)
        {
            ChildInfo &childInfo = childInfos[i];
            FlowInfo &flowInfo = this->flows[childInfo.nFlow];
            double childExtra = (maxHeight-flowInfo.fixedSpace)/flowInfo.extraCount;
            if (childExtra < 0) childExtra = 0;
            flowInfo.extraSpace = childExtra;
            auto child = childInfos[i].child;
            if (childInfo.wantsExtra)
            {
                if (unconstrainedHeight)
                {
                    childExtra = 60;
                    LogError("Height of Lv2cFlexGridElement is unconstrained, but an element has Stretch height");
                }

                child->Measure(Lv2cSize(0,childExtra),Lv2cSize(available.Width(),childExtra),context);
                auto measure = child->MeasuredSize();
                if (measure.Height() > childExtra)
                {
                    measure.Height(childExtra);
                }
                childInfo.measuredSize = measure;
                if (measure.Width() > flowInfo.flowSecondAxisSize)
                {
                    flowInfo.flowSecondAxisSize = measure.Width();
                }
                flowInfo.fixedSpace += measure.Height();
            }
        }
        // pass 3: fix horizontal measures of all elements on autoWidth columns.
        double fixedColumnWidths = 0;
        int columnExtraCount = 0;
        for (auto&flowInfo: flows)
        {
            if (!flowInfo.childHasStretch)
            {
                fixedColumnWidths += flowInfo.flowSecondAxisSize;
            } else {
                columnExtraCount += 1;
            }
        }
        double columnExtra = 0;
        if (columnExtraCount != 0)
        {
            columnExtra = (available.Width()-fixedColumnWidths)/columnExtraCount;
        }
        if (columnExtra < 0) columnExtra = 0;
        for (size_t i = 0; i < childInfos.size(); ++i)
        {
            auto & childInfo = childInfos[i];
            auto & flowInfo = flows[childInfo.nFlow];
            if (flowInfo.childHasStretch)
            {
                flowInfo.flowSecondAxisSize = columnExtra;
            }
            if (childInfo.measuredSize.Width() > flowInfo.flowSecondAxisSize)
            {
                childInfo.measuredSize.Width(flowInfo.flowSecondAxisSize);
            }
        }

        // calculate the measure.
        double width= 0, height=0;
        for (size_t nFlow = 0; nFlow < flows.size(); ++nFlow)
        {
            FlowInfo &flow = flows[nFlow];
            width += flow.flowSecondAxisSize;
            if (flow.fixedSpace > height)
            {
                height = flow.fixedSpace;
            }
        }
        if (flows.size() != 0)
        {
            width += (flows.size()-1)*flexColumnGap;
        }
        if (width > available.Width() ) {
            width = available.Width();
            clipChildren = true;
        }
        if (height > available.Height())
        {
            height = available.Height();
            clipChildren = true;
        }
        Lv2cSize result { width,height};
        return result;
    }
}



Lv2cSize Lv2cFlexGridElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    Lv2cSize borderSize = this->removeThickness(available,style.Margin());
    Lv2cSize paddingSize = this->removeThickness(borderSize,style.BorderWidth());
    Lv2cSize clientSize = this->removeThickness(paddingSize,style.Padding());

    double columnGap = Style().ColumnGap().PixelValue();
    double rowGap = Style().RowGap().PixelValue();
    if (style.FlexDirection() == Lv2cFlexDirection::Row)
    {
        double top = 0;
        for (size_t nFlow = 0; nFlow < flows.size(); ++nFlow)
        {
            FlowInfo &flowInfo = flows[nFlow];
            double bottom = flowInfo.flowSecondAxisSize + top;
            double fixedRowWidth = 0;
            for (size_t nChild = flowInfo.childStart; nChild < flowInfo.childEnd; ++nChild)
            {
                auto &childInfo = childInfos[nChild];
                fixedRowWidth += childInfo.measuredSize.Width();
            }
            double columnExtra = 0;
            double left = 0;
            size_t nChildren = flowInfo.childEnd-flowInfo.childStart;
            double gapExtra = 0;
            if (nChildren > 1)
            {
                gapExtra = (nChildren-1)*columnGap;
            }
            fixedRowWidth += gapExtra;
            double extra = clientSize.Width()-fixedRowWidth;
            auto justification = Style().FlexJustification();
            if (extra < 0) // how to handle overflow?
            {
                extra = 0;
                switch (Style().FlexOverflowJustification())
                {
                    case Lv2cFlexOverflowJustification::Normal:
                        // do nothing.
                        break;
                    case Lv2cFlexOverflowJustification::Start:
                        justification = Lv2cFlexJustification::Start;
                        break;
                    case Lv2cFlexOverflowJustification::End:
                        justification = Lv2cFlexJustification::End;
                        break;
                    case Lv2cFlexOverflowJustification::Center:
                        justification = Lv2cFlexJustification::Center;
                        break;
                }
            }
            switch (justification)
            {
                case Lv2cFlexJustification::Center:
                    left = (clientSize.Width()-fixedRowWidth)/2;
                    break;

                case Lv2cFlexJustification::Start:
                    left = 0;
                    break;
                case Lv2cFlexJustification::End:
                    left = clientSize.Width()-fixedRowWidth;
                    break;
                case Lv2cFlexJustification::SpaceBetween:
                    left = 0;
                    if (nChildren > 1)
                    {
                        columnExtra = (extra) / (nChildren-1);
                    }
                    break;
                case Lv2cFlexJustification::SpaceAround:
                {
                    if (nChildren != 0)
                    {
                        columnExtra = extra / (nChildren+1);
                        left = columnExtra;
                    } 
                    break;
                default:
                    throw std::runtime_error("Unhandled case.");
                }

            }
            for (size_t i = flowInfo.childStart; i < flowInfo.childEnd; ++i)
            {
                auto&child = childInfos[i].child;
                ChildInfo childInfo = childInfos[i];
                double right = left + childInfo.measuredSize.Width();
                Lv2cAlignment alignment = this->Style().FlexAlignItems();


                double layoutHeight = bottom-top;
                double childHeight = childInfo.measuredSize.Height();
                double extra = layoutHeight-childHeight;
                if (extra < 0) extra = 0;

                double childTop = 0, childBottom = 0;
                switch (alignment)
                {
                    case Lv2cAlignment::Start:
                        childTop = top;
                        childBottom = top+childHeight;
                        break;
                    case Lv2cAlignment::End:
                        childBottom = bottom;
                        childTop = childBottom-childHeight;
                        break;
                    case Lv2cAlignment::Center:
                        childTop = top + extra/2;
                        childBottom  = childTop+childHeight;
                        break;
                    case Lv2cAlignment::Stretch:
                        childTop = top;
                        childBottom = bottom;
                        break;
                    default:
                        throw std::runtime_error("Unhandled case.");
                }
                Lv2cRectangle rc { left,childTop, right-left,childBottom-childTop};

                child->Arrange(Lv2cSize(rc.Width(),rc.Height()),context);
                child->Layout(rc);
                left = right + columnExtra + columnGap;
            }
            top = bottom + rowGap;

        }
    } else {
        // column flow.
        // xxx
        double left = 0;
        for (size_t nFlow = 0; nFlow < flows.size(); ++nFlow)
        {
            FlowInfo &flowInfo = flows[nFlow];
            size_t nChildren = flowInfo.childEnd-flowInfo.childStart;

            double right = flowInfo.flowSecondAxisSize + left;
            double fixedColumnHeight = 0;
            for (size_t nChild = flowInfo.childStart; nChild < flowInfo.childEnd; ++nChild)
            {
                auto &childInfo = childInfos[nChild];
                fixedColumnHeight += childInfo.measuredSize.Height();
            }
            if (nChildren != 0)
            {
                fixedColumnHeight += this->flexRowGap * nChildren;
            }
            double rowExtra = 0;
            double top = 0;
            double extra = clientSize.Height()-fixedColumnHeight;

            auto justification = Style().FlexJustification();
            if (extra < 0) 
            {
                extra = 0;
                switch (Style().FlexOverflowJustification())
                {
                    case Lv2cFlexOverflowJustification::Normal:
                        // do nothing.
                        break;
                    case Lv2cFlexOverflowJustification::Start:
                        justification = Lv2cFlexJustification::Start;
                        break;
                    case Lv2cFlexOverflowJustification::End:
                        justification = Lv2cFlexJustification::End;
                        break;
                    case Lv2cFlexOverflowJustification::Center:
                        justification = Lv2cFlexJustification::Center;
                        break;
                }
            }

            switch (justification)
            {
                case Lv2cFlexJustification::Center:
                    top = (extra)/2;
                    break;

                case Lv2cFlexJustification::Start:
                    top = 0;
                    break;
                case Lv2cFlexJustification::End:
                    top = extra;
                    break;
                case Lv2cFlexJustification::SpaceBetween:
                    top = 0;
                    if (nChildren > 1)
                    {
                        rowExtra = (extra) / (nChildren-1);
                    }
                    break;
                case Lv2cFlexJustification::SpaceAround:
                {
                    if (nChildren != 0)
                    {
                        rowExtra = extra / (nChildren+1);
                        top = rowExtra;
                    } 
                    break;
                }

            }
            for (size_t i = flowInfo.childStart; i < flowInfo.childEnd; ++i)
            {
                auto&child = childInfos[i].child;
                ChildInfo childInfo = childInfos[i];
                double bottom = top + childInfo.measuredSize.Height();
                Lv2cAlignment alignment = this->Style().FlexAlignItems();

                double layoutWidth = right-left;
                double childWidth = childInfo.measuredSize.Width();
                double extra = layoutWidth-childWidth;
                if (extra < 0) extra = 0;

                double childLeft, childRight;
                switch (alignment)
                {
                    case Lv2cAlignment::Start:
                        childLeft = left;
                        childRight = left+childWidth;
                        break;
                    case Lv2cAlignment::End:
                        childRight = right;
                        childLeft = right-childWidth;
                        break;
                    case Lv2cAlignment::Center:
                        childLeft = left + extra/2;
                        childRight = childLeft+childWidth;
                        break;
                    case Lv2cAlignment::Stretch:
                        childLeft = left;
                        childRight = right;
                        break;
                    default:
                        throw std::runtime_error("Unhandled case.");
                }
                Lv2cRectangle rc { childLeft,top, childRight-childLeft,bottom-top};

                child->Arrange(Lv2cSize(rc.Width(),rc.Height()),context);
                child->Layout(rc);
                top = bottom + rowExtra + rowGap;
            }

            left = right + columnGap;
        }

    }
    for (auto&child: invisibleChildren)
    {
        child->Layout(Lv2cRectangle(-50000,-50000,0,0));
    }
    return available;
}


void Lv2cFlexGridElement::Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) 
{
    Lv2cElement::Measure(constraint,maxAvailable,context);
}


bool Lv2cFlexGridElement::ClipChildren() const 
{
    return clipChildren;
}


