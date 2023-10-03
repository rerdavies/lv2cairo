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

#include "LvtkContainerElement.hpp"
#include <vector>


namespace lvtk {
    struct LvtkColumnDefinition {
        LvtkAlignment rowAlignment = LvtkAlignment::Start;
        LvtkAlignment columnAlignment = LvtkAlignment::Start;
        double columnWidth = 0;
    };
    class LvtkTableElement : public LvtkContainerElement {
    public:
        using self = LvtkTableElement;
        using super = LvtkElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

        LvtkElement::ptr&Child(size_t row, size_t column);


#pragma GCC diagnostic pop
    public:
        size_t ColumnCount() const;
        LvtkTableElement &ColumnCount(size_t columns);
        size_t RowCount() const;

        std::vector<LvtkColumnDefinition> &ColumnDefinitions();
        LvtkTableElement& ColumnDefinitions(const std::vector<LvtkColumnDefinition> &columnDefinitions);
        LvtkTableElement& ColumnDefinitions(std::vector<LvtkColumnDefinition> &&columnDefinitions);

        LvtkTableElement&AddRow(const std::vector<LvtkElement::ptr>&children);
        LvtkTableElement&AddRow(std::vector<LvtkElement::ptr>&&children);

    public:
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;
    protected:
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;

        virtual void OnDraw(LvtkDrawingContext &dc) override;

    private:
        std::vector<LvtkColumnDefinition> columnDefinitions;

        std::vector<double> columnWidths;
        std::vector<double> rowHeights;
    };
}