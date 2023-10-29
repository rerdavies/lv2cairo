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

#include "lv2c/Lv2cContainerElement.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"

namespace lvtk
{
    class Lv2cTypographyElement;
    class Lv2cFlexGridElement;
}

namespace lvtk {
    class Lv2cGroupElement: public Lv2cContainerElement 
    {
    public:
        using self = Lv2cGroupElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cGroupElement();
        

        BINDING_PROPERTY_REF(Text,std::string,"")

        virtual void AddChild(std::shared_ptr<Lv2cElement> child) override;
        virtual void AddChild(std::shared_ptr<Lv2cElement> child,size_t position) override;
        virtual bool RemoveChild(std::shared_ptr<Lv2cElement> element) override;
        virtual void RemoveChild(size_t index) override;
        virtual size_t ChildCount() const override;
        virtual Lv2cElement::ptr Child(size_t index) override ;
        virtual const Lv2cElement *Child(size_t index) const override;
        virtual std::vector<Lv2cElement::ptr> &Children() override;

        virtual void Children(const std::vector<Lv2cElement::ptr>&children) override;
        virtual void RemoveAllChildren() override;



    public:
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override;




    protected:
        virtual void OnDraw(Lv2cDrawingContext &dc) override;
        virtual void OnMount() override;

    private:

        std::shared_ptr<Lv2cTypographyElement> typography;
        std::shared_ptr<Lv2cFlexGridElement> flexGrid;
    };
}