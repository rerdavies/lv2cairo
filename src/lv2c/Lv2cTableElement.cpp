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

#include "lv2c/Lv2cTableElement.hpp"
#include "lv2c/Lv2cLog.hpp"

using namespace lv2c;

Lv2cTableElement &Lv2cTableElement::AddRow(const std::vector<Lv2cElement::ptr> &children)
{
    auto &c = Children();
    c.insert(c.end(), children.begin(), children.end());
    return *this;
}
Lv2cTableElement &Lv2cTableElement::AddRow(std::vector<Lv2cElement::ptr> &&children)
{
    auto &c = Children();
    c.insert(c.end(), std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
    return *this;
}

std::vector<Lv2cColumnDefinition> &Lv2cTableElement::ColumnDefinitions()
{
    return columnDefinitions;
}
Lv2cTableElement &Lv2cTableElement::ColumnDefinitions(const std::vector<Lv2cColumnDefinition> &columnDefinitions)
{
    this->columnDefinitions = columnDefinitions;
    return *this;
}

Lv2cTableElement &Lv2cTableElement::ColumnDefinitions(std::vector<Lv2cColumnDefinition> &&columnDefinitions)
{
    this->columnDefinitions = std::move(columnDefinitions);
    return *this;
}

size_t Lv2cTableElement::ColumnCount() const
{
    return columnDefinitions.size();
}
Lv2cTableElement &Lv2cTableElement::ColumnCount(size_t columns)
{
    columnDefinitions.resize(columns);
    return *this;
}

size_t Lv2cTableElement::RowCount() const
{
    if (ColumnCount() == 0)
    {
        return 0;
    }
    return ChildCount() / ColumnCount();
}

Lv2cElement::ptr &Lv2cTableElement::ChildCell(size_t row, size_t column)
{
    return Children()[row * ColumnCount() + column];
}

Lv2cSize Lv2cTableElement::MeasureClient(
    Lv2cSize clientConstraint, 
    Lv2cSize clientAvailable, 
    Lv2cDrawingContext &context)
{

    size_t rowCount = RowCount();
    size_t columnCount = ColumnCount();

    auto borderThickness = Style().BorderWidth().PixelValue();
    auto cellPadding = Style().CellPadding().PixelValue();

    Lv2cSize cellExtra { cellPadding.left+cellPadding.right,cellPadding.top+cellPadding.bottom} ;
    double rowGap = borderThickness.top;
    double columnGap = borderThickness.left;

    this->rowHeights.resize(rowCount);
    this->columnWidths.resize(columnCount);

    // pass 1: measure all auto and fixed-width columns.
    double totalWeights = 0;
    double fixedWidths = 0;
    for (size_t c = 0; c < columnCount; ++c)
    {
        auto &columnDefinition = columnDefinitions[c];
        if (columnDefinition.columnAlignment == Lv2cAlignment::Stretch)
        {
            columnWidths[c] = -1;
            double weight = columnDefinition.columnWidth;
            if (weight == 0)
                weight = 1;
            totalWeights += weight;
        }
        else
        {

            Lv2cSize childAvailable = clientAvailable;

            if (columnDefinition.columnWidth != 0)
            {
                Lv2cSize childConstraint(columnDefinition.columnWidth- cellExtra.Width(), 0);
                for (size_t r = 0; r < rowCount; ++r)
                {
                    auto &child = ChildCell(r, c);
                    child->Measure(childConstraint, childAvailable, context);
                    Lv2cSize size = child->MeasuredSize();
                    size = size + cellExtra;
                    if (size.Height() > rowHeights[r])
                    {
                        rowHeights[r] = size.Height();
                    }
                }
                fixedWidths += columnDefinition.columnWidth;
                columnWidths[c] = columnDefinition.columnWidth;
            }
            else
            {
                // auto width.
                Lv2cSize childConstraint(0, 0);
                double maxWidth = 0;

                for (size_t r = 0; r < rowCount; ++r)
                {
                    auto &child = ChildCell(r, c);
                    child->Measure(childConstraint, childAvailable, context);
                    auto size = child->MeasuredSize();
                    size = size + cellExtra;
                    if (size.Width() > maxWidth)
                    {
                        maxWidth = size.Width();
                    }
                    if (size.Height() > rowHeights[r])
                    {
                        rowHeights[r] = size.Height();
                    }
                }
                fixedWidths += maxWidth;
                columnWidths[c] = maxWidth;
            }
        }
    }
    if (totalWeights != 0)
    {
        // divide out the extra space, and do measure of stretch columns.
        double extra = 0;
        if (clientConstraint.Width() == 0)
        {
            LogError("Table has stretch columns, but table width is not stretchy.");
            extra = 10;
        }
        else
        {
            extra = clientConstraint.Width() - fixedWidths;
        }
        extra -= (columnCount-1)*(columnGap);
        if (extra < 0)
            extra = 0;
        double dExtra = extra / totalWeights;

        for (size_t c = 0; c < columnCount; ++c)
        {
            auto &columnDefinition = columnDefinitions[c];
            if (columnDefinition.columnAlignment == Lv2cAlignment::Stretch)
            {
                double columnWidth = columnDefinition.columnWidth == 0
                                            ? dExtra
                                            : columnDefinition.columnWidth * dExtra;
                columnWidths[c] = columnWidth;
                double  childWidth = columnWidth-cellExtra.Width();
                if (childWidth < 0)
                {
                    childWidth = 0;
                }
                Lv2cSize childConstraint(childWidth, 0);

                Lv2cSize childAvailable(childWidth,clientAvailable.Height());
                for (size_t r = 0; r < rowCount; ++r)
                {
                    auto &child = ChildCell(r, c);

                    child->Measure(childConstraint, childAvailable, context);
                    auto measureSize = child->MeasuredSize();
                    measureSize = measureSize + cellExtra;
                    if (measureSize.Height() > rowHeights[r])
                    {
                        rowHeights[r] = measureSize.Height();
                    }
                }
            }
        }
    }
    double width = 0, height = 0;
    for (size_t c = 0; c < columnCount; ++c)
    {
        width += columnWidths[c];
    }
    for (size_t r = 0; r < rowCount; ++r)
    {
        height += rowHeights[r];
    }
    if (columnCount != 0)
    {
        width += (columnCount-1)*columnGap;
    }
    if (rowCount != 0)
    {
        height += (rowCount-1)*rowGap;
    }
    return Lv2cSize{width, height};
}

Lv2cSize Lv2cTableElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    size_t rowCount = RowCount();
    size_t columnCount = ColumnCount();
    double y = 0;

    auto borderThickness = Style().BorderWidth().PixelValue();
    auto cellPadding = Style().CellPadding().PixelValue();

    Lv2cSize cellExtra { cellPadding.left+cellPadding.right,cellPadding.top+cellPadding.bottom} ;
    double rowGap = borderThickness.top;
    double columnGap = borderThickness.left;

    for (size_t r = 0; r < rowCount; ++r)
    {
        double x = 0;
        for (size_t c = 0; c < columnCount; ++c)
        {
            auto &child = ChildCell(r, c);
            Lv2cSize measuredSize = child->MeasuredSize();
            auto &columnDefinition = columnDefinitions[c];

            Lv2cSize childSize{measuredSize.Width(), measuredSize.Height()};
            child->Arrange(available, context);
            double childX,childY;
            switch (columnDefinition.columnAlignment) {
                case Lv2cAlignment::Start:
                    childX = x  + cellPadding.left;
                    break;
                case Lv2cAlignment::End:
                    childX = x + columnWidths[c]-cellPadding.right-childSize.Width();
                    break;
                case Lv2cAlignment::Center:
                    childX = x + cellPadding.left + (columnWidths[c]-cellExtra.Width()-childSize.Width())/2;
                    break;

                case Lv2cAlignment::Stretch:
                    childX = x + cellPadding.left;
                    break;
                default:
                    throw std::runtime_error("Invalid column alignment.");
            }
            switch (columnDefinition.rowAlignment)
            {
                case Lv2cAlignment::Start:
                    childY = y + cellPadding.top;
                    break;
                case Lv2cAlignment::End:
                    childY = y + rowHeights[r]- cellPadding.bottom-childSize.Height();
                    break;
                case Lv2cAlignment::Center:
                    childY = y + cellPadding.top + (rowHeights[r]-cellExtra.Height()-childSize.Height())/2;
                    break;
                case Lv2cAlignment::Stretch:
                    throw std::runtime_error("Not supported: ColumnDefintion::rowAlignment == LvktAlignment::Stretch");
                default:
                    throw std::runtime_error("Invalid row alignment.");
            }
            Lv2cRectangle bounds{childX, childY, childSize.Width(), childSize.Height()};
            child->Layout(bounds);
            x += columnWidths[c] + columnGap;
        }
        y += rowHeights[r] + rowGap;
    }
    return available;
}
void Lv2cTableElement::OnDraw(Lv2cDrawingContext &dc) 
{
    // only gets called if border is non-empty. (see Lv2cElement::WillDraw())
    super::OnDraw(dc);

    size_t rowCount = RowCount();
    size_t columnCount = ColumnCount();

    dc.set_source(Style().BorderColor());
    auto borderWidth = Style().BorderWidth().PixelValue();
    auto size = ClientSize();

    Lv2cRectangle rc { 0,0,size.Width(), borderWidth.top};

    double y = 0;
    for (size_t r = 0; r < rowCount-1; ++r)
    {
        y += rowHeights[r];
        rc.Top(y);
        dc.rectangle(rc);
        dc.fill();
        y += borderWidth.top;
    }

    double x = 0;
    rc = Lv2cRectangle(0,0,borderWidth.left,size.Height());
    for (size_t c = 0; c < columnCount-1; ++c)
    {
        x += columnWidths[c];
        rc.Left(x);
        dc.rectangle(rc);
        dc.fill();
        x += borderWidth.left;
    }
}
