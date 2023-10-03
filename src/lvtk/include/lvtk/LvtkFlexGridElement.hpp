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

namespace lvtk {    

    class LvtkFlexGridElement: public LvtkContainerElement {
    public:
        virtual const char* Tag() const override { return "FlexGrid";}

        using self = LvtkFlexGridElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        static ptr Create(LvtkStyle &&style) { return std::make_shared<LvtkFlexGridElement>(std::move(style)); }
        static ptr Create(LvtkStyle &&style, const std::vector<LvtkElement::ptr> &children) { return std::make_shared<LvtkFlexGridElement>(std::move(style),children); }
        LvtkFlexGridElement() { }
        LvtkFlexGridElement(LvtkStyle&&style) { Style(style);}
        LvtkFlexGridElement(LvtkStyle&&style, const std::vector<LvtkElement::ptr> &children) { Style(style); this->Children(children); }


    public:
        
        virtual LvtkSize MeasureClient(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override;
        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override;
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;
    protected:
        virtual bool ClipChildren() const override;

    private:
        bool clipChildren = false;
        //LvtkFlexGridElement() { }
        //LvtkFlexGridElement(LvtkStyle&&style) { Style(style);}
        LvtkRectangle clientMeasure;
        double flexRowGap = 0;
        double flexColumnGap = 0;

        struct ChildInfo {
            LvtkElement* child;
            LvtkSize measuredSize;
            bool wantsExtra = false;
            size_t nFlow = 0;
            bool hasMeasure = false;
        };
        std::vector<LvtkElement*> invisibleChildren;
        std::vector<ChildInfo> childInfos;
        struct FlowInfo {
            size_t childStart = 0;
            size_t childEnd = 0;
            double space = 0;
            double fixedSpace = 0;
            double extraSpace = 0;
            double extraCount = 0;
            bool childHasStretch = false;
            double flowSecondAxisSize = 0;
            double nFlow = 0;
        };
        std::vector<FlowInfo> flows;
    };
} // namespace