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
#include "lv2c/Lv2cTypographyElement.hpp"
#include <memory>



namespace lv2c::ui
{

    class Lv2PortInfo;
    
    /// @brief Display text from a list of items.
    /// Behaves like Lv2cDropdownElement, but read-only. 
    /// 
    /// Display the text from a list of Lv2cDropdownItem's based on the value 
    /// of the SelectedId property.
    class Lv2TextOutputElement : public Lv2cContainerElement
    {
    public:
        using super = Lv2cContainerElement;
        using self = Lv2TextOutputElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(const Lv2PortInfo*portInfo) { return std::make_shared<self>(portInfo); }
        Lv2TextOutputElement(const Lv2PortInfo*portInfo);
        virtual ~Lv2TextOutputElement() noexcept;


        Lv2cBindingProperty<std::string> DisplayValueProperty;
        const std::string& DisplayValue() { return DisplayValueProperty.get(); }
        Lv2TextOutputElement &DisplayValue(const std::string&value) { DisplayValueProperty.set(value); return *this; }



    protected:
        virtual void OnMount() override;

        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override;

        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context) override
        {
            super::Measure(constraint, maxAvailable, context);
        }


    private:
        const  Lv2PortInfo*portInfo = nullptr;

        std::shared_ptr<Lv2cTypographyElement> typography;

    };
}