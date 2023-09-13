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

#pragma once

#include "LvtkContainerElement.hpp"

namespace lvtk {

    class LvtkVerticalStackElement : public LvtkContainerElement
    {
    public:
        virtual const char* Tag() const override { return "VStack";}

        using super = LvtkContainerElement;

        using ptr = std::shared_ptr<LvtkVerticalStackElement>;

        static ptr Create() { return std::make_shared<LvtkVerticalStackElement>(); }

        LvtkVerticalStackElement();
        virtual ~LvtkVerticalStackElement();

    protected:
        virtual bool ClipChildren() const override;
        virtual void Measure(LvtkSize constraint,LvtkSize available, LvtkDrawingContext &context) override;
        virtual LvtkSize MeasureClient(LvtkSize constraint,LvtkSize available, LvtkDrawingContext &context) override;
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;
    private:
        bool clipChildren = false;
        struct ChildInfo {
            LvtkSize measureSize;
            LvtkSize arrangeSize;
        };
        std::vector<ChildInfo> childInfos;
        //StyleContext GetHorizontalMeasureContext(double width, LvtkElement*element);
    };

}// namespace