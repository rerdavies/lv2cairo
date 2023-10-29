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

namespace lvtk {    

    class Lv2cFlexGridElement: public Lv2cContainerElement {
    public:
        virtual const char* Tag() const override { return "FlexGrid";}

        using self = Lv2cFlexGridElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }
        static ptr Create(Lv2cStyle &&style) { return std::make_shared<Lv2cFlexGridElement>(std::move(style)); }
        static ptr Create(Lv2cStyle &&style, const std::vector<Lv2cElement::ptr> &children) { return std::make_shared<Lv2cFlexGridElement>(std::move(style),children); }
        Lv2cFlexGridElement() { }
        Lv2cFlexGridElement(Lv2cStyle&&style) { Style(style);}
        Lv2cFlexGridElement(Lv2cStyle&&style, const std::vector<Lv2cElement::ptr> &children) { Style(style); this->Children(children); }


    public:
        
        virtual Lv2cSize MeasureClient(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override;
        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override;
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;
    protected:
        virtual bool ClipChildren() const override;

    private:
        bool clipChildren = false;
        //Lv2cFlexGridElement() { }
        //Lv2cFlexGridElement(Lv2cStyle&&style) { Style(style);}
        Lv2cRectangle clientMeasure;
        double flexRowGap = 0;
        double flexColumnGap = 0;

        struct ChildInfo {
            Lv2cElement* child;
            Lv2cSize measuredSize;
            bool wantsExtra = false;
            size_t nFlow = 0;
            bool hasMeasure = false;
        };
        std::vector<Lv2cElement*> invisibleChildren;
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