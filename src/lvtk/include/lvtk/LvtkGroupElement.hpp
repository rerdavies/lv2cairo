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

#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkBindingProperty.hpp"

namespace lvtk
{
    class LvtkTypographyElement;
    class LvtkFlexGridElement;
}

namespace lvtk {
    class LvtkGroupElement: public LvtkContainerElement 
    {
    public:
        using self = LvtkGroupElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkGroupElement();
        

        BINDING_PROPERTY_REF(Text,std::string,"")

        virtual void AddChild(std::shared_ptr<LvtkElement> child) override;
        virtual void AddChild(std::shared_ptr<LvtkElement> child,size_t position) override;
        virtual bool RemoveChild(std::shared_ptr<LvtkElement> element) override;
        virtual void RemoveChild(size_t index) override;
        virtual size_t ChildCount() const override;
        virtual LvtkElement::ptr Child(size_t index) override ;
        virtual const LvtkElement *Child(size_t index) const override;
        virtual std::vector<LvtkElement::ptr> &Children() override;

        virtual void Children(const std::vector<LvtkElement::ptr>&children) override;
        virtual void RemoveAllChildren() override;



    public:
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;




    protected:
        virtual void OnDraw(LvtkDrawingContext &dc) override;
        virtual void OnMount() override;

    private:

        std::shared_ptr<LvtkTypographyElement> typography;
        std::shared_ptr<LvtkFlexGridElement> flexGrid;
    };
}