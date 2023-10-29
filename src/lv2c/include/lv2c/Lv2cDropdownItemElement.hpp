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
#include <memory>

#include "Lv2cButtonBaseElement.hpp"
namespace lvtk
{
    class Lv2cDropdownItemElement: public Lv2cButtonBaseElement {
    public:
        using self = Lv2cDropdownItemElement;
        using super = Lv2cButtonBaseElement;
        using ptr = std::shared_ptr<self>;
        using selection_id_t = int64_t;
        static ptr Create(
            selection_id_t selectionId,
            const std::string& text, 
            const std::string& svgIcon) { return std::make_shared<self>(selectionId,text,svgIcon,true); }
            

        static ptr Create(
            int64_t selectionId,
            const std::string& text) { return std::make_shared<self>(selectionId,text,"",false); }
            
            
        Lv2cDropdownItemElement(selection_id_t selectionId,const std::string& text, const std::string&svgIcon,bool hasIcon);

        selection_id_t SelectionId() const;
    protected:
        virtual const Lv2cHoverColors &HoverBackgroundColors() override;

        virtual void OnMount() override;
        virtual void OnHoverStateChanged(Lv2cHoverState hoverState) override;
        virtual bool ShowPressedState() const override;

    private:
        selection_id_t selectionId;
        Lv2cElement::ptr icon;
        Lv2cElement::ptr typography;
        std::string text;
        std::string svgIcon;
        bool hasIcon;
        Lv2cHoverColors hoverTextColors;
    };
}

