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

#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include <vector>

namespace lvtk {
    class LvtkTheme;
}

namespace lvtk::ui
{
    class Lv2PortViewController;

    class Lv2PortViewFactory
    {
    public:
        using self = Lv2PortViewFactory;
        using ptr = std::shared_ptr<self>;

        virtual ~Lv2PortViewFactory() { }
        LvtkTheme&Theme();

        Lv2PortViewFactory& Theme(std::shared_ptr<LvtkTheme> theme);

        static ptr Create() { return std::make_shared<self>(); }

        LvtkSize DefaultSize() const;
        // Height of the edit/label area.
        virtual double EditControlHeight() const;
        // With of the port control, dependent on control type.
        virtual double GetControlWidth(Lv2PortViewController *viewController, double defaultControlWidth);

        virtual LvtkContainerElement::ptr CreatePage();

        virtual LvtkContainerElement::ptr CreateContainer(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateCaption(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateControl(Lv2PortViewController *viewController);

        virtual LvtkElement::ptr CreateDial(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateDropdown(Lv2PortViewController *viewController, const std::vector<LvtkDropdownItem> &items);
        virtual LvtkElement::ptr CreateToggle(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateOnOff(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateVuMeter(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateStereoVuMeter(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateEditArea(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateLed(Lv2PortViewController *viewController);
        virtual LvtkElement::ptr CreateStatusMessage(Lv2PortViewController *viewController,const std::vector<LvtkDropdownItem> &items);


        enum class ValueControlStyle
        {
            // editable value.
            EditBox,
            // display current value.
            Label,
            // no editbox, no label, but space reserved.
            Hidden,
            // no space reserved.
            Collapsed
        };

        virtual ValueControlStyle GetValueControlStyle(Lv2PortViewController *viewController);

    private:
        std::shared_ptr<LvtkTheme> theme;
    };

}