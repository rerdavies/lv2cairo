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

#pragma once

#include "Lv2cContainerElement.hpp"
#include <vector>


namespace lv2c {
    struct Lv2cColumnDefinition {
        Lv2cAlignment rowAlignment = Lv2cAlignment::Start;
        Lv2cAlignment columnAlignment = Lv2cAlignment::Start;
        double columnWidth = 0;
    };
    class Lv2cTableElement : public Lv2cContainerElement {
    public:
        using self = Lv2cTableElement;
        using super = Lv2cElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }


        Lv2cElement::ptr&ChildCell(size_t row, size_t column);

    public:
        size_t ColumnCount() const;
        Lv2cTableElement &ColumnCount(size_t columns);
        size_t RowCount() const;

        std::vector<Lv2cColumnDefinition> &ColumnDefinitions();
        Lv2cTableElement& ColumnDefinitions(const std::vector<Lv2cColumnDefinition> &columnDefinitions);
        Lv2cTableElement& ColumnDefinitions(std::vector<Lv2cColumnDefinition> &&columnDefinitions);

        Lv2cTableElement&AddRow(const std::vector<Lv2cElement::ptr>&children);
        Lv2cTableElement&AddRow(std::vector<Lv2cElement::ptr>&&children);

    public:
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;
    protected:
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;

        virtual void OnDraw(Lv2cDrawingContext &dc) override;

    private:
        std::vector<Lv2cColumnDefinition> columnDefinitions;

        std::vector<double> columnWidths;
        std::vector<double> rowHeights;
    };
}