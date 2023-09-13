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
#include "LvtkButtonBaseElement.hpp"


namespace lvtk {
    class LvtkTypographyElement;
    class LvtkSvgElement;

    enum class LvtkButtonVariant {
        Dialog,
        DialogPrimary,
        DialogSecondary,
        BorderButton,
        BorderButtonDefault,
        BorderButtonPrimary,
        BorderButtonSecondary,

        ImageButton,
        ImageButtonPrimary,
        ImageButtonSecondary,
    };

    class LvtkButtonElement: public LvtkButtonBaseElement {
    public:
        virtual const char* Tag() const override { return "Button";}

        using self = LvtkButtonElement;
        using super=LvtkButtonBaseElement;
        using ptr =std::shared_ptr<LvtkButtonElement>;
        static ptr Create() { return std::make_shared<LvtkButtonElement>();}

        LvtkButtonElement();
        LvtkButtonElement&Variant(LvtkButtonVariant variant);        
        LvtkButtonVariant Variant() const;

        BINDING_PROPERTY_REF(Icon,std::string,std::string())
        BINDING_PROPERTY_REF(Text,std::string,std::string())

    protected:
        virtual bool OnKeyDown(const LvtkKeyboardEventArgs&event) override;
        virtual bool OnKeycodeDown(const LvtkKeyboardEventArgs&event) override;
        virtual bool OnKeycodeUp(const LvtkKeyboardEventArgs&event) override;
        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;


        virtual void OnMount() override;



        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override;
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override
        {
            return super::Arrange(available,context);
        }
        virtual bool WantsFocus() const override { 
            return true; 
        }

        virtual bool ShowPressedState() const override;

        virtual const LvtkHoverColors &HoverBackgroundColors() override;


    private:

        void OnTextChanged(const std::string&value);
        void OnIconChanged(const std::string&value);
        bool KeyPressed() const;
        void KeyPressed(bool value);

        void RebuildChildren();

        LvtkButtonVariant variant = LvtkButtonVariant::Dialog;

        bool keyPressed = false;
        LvtkHoverColors hoverColors;
        std::shared_ptr<LvtkTypographyElement> typography;
        std::shared_ptr<LvtkElement> icon;
    };
} // namespace