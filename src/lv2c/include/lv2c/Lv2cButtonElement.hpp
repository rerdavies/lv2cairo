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
#include "Lv2cButtonBaseElement.hpp"


namespace lv2c {
    class Lv2cTypographyElement;
    class Lv2cSvgElement;

    enum class Lv2cButtonVariant {
        Dialog,
        DialogPrimary,
        DialogSecondary,

        BorderButton,
        BorderButtonDefault,
        BorderButtonPrimary,
        BorderButtonSecondary,

        FilledButtonPrimary,
        FilledButtonSecondary,

        ImageButton,
        ImageButtonPrimary,
        ImageButtonSecondary,
    };

    class Lv2cButtonElement: public Lv2cButtonBaseElement {
    public:
        virtual const char* Tag() const override { return "Button";}

        using self = Lv2cButtonElement;
        using super=Lv2cButtonBaseElement;
        using ptr =std::shared_ptr<Lv2cButtonElement>;
        static ptr Create() { return std::make_shared<Lv2cButtonElement>();}

        Lv2cButtonElement();
        Lv2cButtonElement&Variant(Lv2cButtonVariant variant);        
        Lv2cButtonVariant Variant() const;

        BINDING_PROPERTY_REF(Icon,std::string,std::string())
        BINDING_PROPERTY_REF(Text,std::string,std::string())

    protected:
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs&event) override;
        virtual bool OnKeycodeDown(const Lv2cKeyboardEventArgs&event) override;
        virtual bool OnKeycodeUp(const Lv2cKeyboardEventArgs&event) override;
        virtual bool OnMouseDown(Lv2cMouseEventArgs &event) override;
        

        virtual void OnMount() override;



        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable,Lv2cDrawingContext &context) override;
        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context) override
        {
            return super::Arrange(available,context);
        }
        virtual bool WantsFocus() const override { 
            return true; 
        }

        virtual bool ShowPressedState() const override;

        virtual const Lv2cHoverColors &HoverBackgroundColors() override;


    private:

        void OnTextChanged(const std::string&value);
        void OnIconChanged(const std::string&value);
        bool KeyPressed() const;
        void KeyPressed(bool value);

        void RebuildChildren();

        Lv2cButtonVariant variant = Lv2cButtonVariant::Dialog;

        bool keyPressed = false;
        Lv2cHoverColors hoverColors;
        std::shared_ptr<Lv2cTypographyElement> typography;
        std::shared_ptr<Lv2cElement> icon;
    };
} // namespace