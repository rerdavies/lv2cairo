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

#include "lv2c/Lv2cButtonBaseElement.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"

namespace lv2c {
    class Lv2cTypographyElement;
    class Lv2cSvgElement;
}

namespace lv2c::ui {
    using namespace lv2c;

    class Lv2FileElement: public Lv2cButtonBaseElement {
    public: 
        using self=Lv2FileElement;
        using super=Lv2cElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2FileElement();

        BINDING_PROPERTY(PatchProperty,std::string,"");

        BINDING_PROPERTY(Filename,std::string,"");
    protected:

        virtual void OnHoverStateChanged(Lv2cHoverState hoverState) override;

        virtual bool WantsFocus() const override;
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs&event) override;

        virtual void OnMount() override;


    private:
        void UpdateColors();

        void OnFilenameChanged(const std::string&filename);
        BINDING_PROPERTY(DropdownText,std::string,"");

        Lv2cHoverColors hoverTextColors;

        std::shared_ptr<Lv2cTypographyElement> typography;
        std::shared_ptr<Lv2cSvgElement> icon;
    };
}