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

#include "lvtk_ui/Lv2PortView.hpp"
#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkButtonBaseElement.hpp"
#include "lvtk/LvtkNumericEditBoxElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk_ui/Lv2ControlConstants.hpp"

#define XK_MISCELLANY
#include "X11/keysymdef.h"

using namespace lvtk;

namespace lvtk::ui
{
    class Lv2PortElement : public LvtkStereoValueElement
    {
    public:
        using ptr = std::shared_ptr<Lv2PortElement>;
        using super = LvtkContainerElement;
        static ptr Create(
            Lv2PortViewController::ptr viewController,
            Lv2PortViewFactory::ptr controlFactory) { return std::make_shared<Lv2PortElement>(viewController, controlFactory); }

        Lv2PortElement(Lv2PortViewController::ptr viewController, Lv2PortViewFactory::ptr controlFactory);
        virtual ~Lv2PortElement();

    protected:

        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override
        {
            LvtkSize result = super::Arrange(available,context);


            return result;
        }

    private:
        Lv2PortViewFactory::ptr controlFactory;


        std::string NiceEditText(double value) const;
        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable, LvtkDrawingContext &context) override
        {
            return super::Measure(constraint, maxAvailable, context);
        }
        Lv2PortViewController::ptr viewcontroller;
    };

    Lv2PortElement::~Lv2PortElement()
    {
    }

    Lv2PortElement::Lv2PortElement(
        Lv2PortViewController::ptr viewController,
        Lv2PortViewFactory::ptr controlFactory)
        : viewcontroller(viewController), controlFactory(controlFactory)
    {
        viewController->PortValueProperty.Bind(this->ValueProperty);
        viewController->RightPortValueProperty.Bind(this->RightValueProperty);
        
        auto containerElement = controlFactory->CreateContainer(viewController.get());
        this->AddChild(containerElement);

        auto caption = controlFactory->CreateCaption(viewController.get());
        containerElement->AddChild(caption);

        auto control = controlFactory->CreateControl(viewController.get());
        auto stretchElement = LvtkContainerElement::Create();
        stretchElement->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch);
        stretchElement->AddChild(control);
        containerElement->AddChild(stretchElement);

        containerElement->AddChild(controlFactory->CreateEditArea(viewController.get()));
        Stereo(viewController->GetViewType() == Lv2PortViewType::StereoVuMeter);
    }

    LvtkValueElement::ptr CreatePortView(
        const Lv2PortInfo &portInfo,
        std::shared_ptr<Lv2PortViewFactory> controlFactory)
    {
        if (!controlFactory)
        {
            controlFactory = Lv2PortViewFactory::Create();
        }
        Lv2PortViewController::ptr viewcontroller = std::make_shared<Lv2PortViewController>(portInfo);
        return Lv2PortElement::Create(viewcontroller, controlFactory);
    }
    LvtkValueElement::ptr CreatePortView(
        LvtkBindingProperty<double> &value,
        const Lv2PortInfo &portInfo,
        std::shared_ptr<Lv2PortViewFactory> controlFactory)
    {
        if (!controlFactory)
        {
            controlFactory = Lv2PortViewFactory::Create();
        }
        Lv2PortViewController::ptr viewcontroller = std::make_shared<Lv2PortViewController>(portInfo);
        value.Bind(viewcontroller->PortValueProperty);
        return Lv2PortElement::Create(viewcontroller, controlFactory);
    }
    LvtkStereoValueElement::ptr CreateStereoPortView(
        const std::string& label,
        LvtkBindingProperty<double> &leftValue,
        LvtkBindingProperty<double> &rightValue,
        const Lv2PortInfo &portInfo,
        std::shared_ptr<Lv2PortViewFactory> controlFactory)
    {
        if (!controlFactory)
        {
            controlFactory = Lv2PortViewFactory::Create();
        }
        Lv2PortInfo labelInfo { portInfo};
        labelInfo.name(label);
        Lv2PortViewController::ptr viewcontroller = std::make_shared<Lv2PortViewController>(labelInfo);
        leftValue.Bind(viewcontroller->PortValueProperty);
        rightValue.Bind(viewcontroller->RightPortValueProperty);
        return Lv2PortElement::Create(viewcontroller, controlFactory);
    }

    std::string Lv2PortElement::NiceEditText(double value) const
    {
        std::stringstream s;
        if (viewcontroller->IsInteger())
        {
            s << (int64_t)value;
            return s.str();
        }
        s.precision(15);
        s << value;
        std::string t = s.str();

        auto i = t.begin();
        if (i != t.end() && (*i) == '-')
            ++i;
        int nonZeroDigits = 0;
        bool seenDecimalPoint = false;
        while (i != t.end())
        {
            char c = *i;
            if (c == 'E' || c == 'e')
            {
                // scientific. :-/ Display it all.
                i = t.end();
                break;
            }
            else if (c >= '0' && c <= '9')
            {
                if (c != '0' || nonZeroDigits != 0)
                {
                    ++nonZeroDigits;
                }
                if (nonZeroDigits >= 3 + 1 && seenDecimalPoint)
                {
                    break;
                }
            }
            else if (c == '.' || c == ',')
            {
                seenDecimalPoint = true;
                if (nonZeroDigits >= 3)
                {
                    break;
                }
            }
            else
            {
                i = t.end(); // non-numeric. (nan??). show it all.
                break;
            }
            ++i;
        }
        return std::string(t.begin(), i);
    }


        
}
