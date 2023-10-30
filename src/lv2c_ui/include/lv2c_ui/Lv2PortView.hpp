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

#include "lv2c/Lv2cElement.hpp"
#include "lv2c/Lv2cValueElement.hpp"
#include "Lv2PluginInfo.hpp"

#include "Lv2PortViewController.hpp"
#include  "lv2c/Lv2cContainerElement.hpp"
#include "Lv2PortViewFactory.hpp"

namespace lv2c {
    class Lv2cDropdownItem;
}
namespace lv2c::ui {
    class Lv2PortViewController;

   
    Lv2cValueElement::ptr CreatePortView(
        const Lv2PortInfo&portInfo,
        std::shared_ptr<Lv2PortViewFactory> customization = nullptr);


    Lv2cValueElement::ptr CreatePortView(
        Lv2cBindingProperty<double> &value,
        const Lv2PortInfo&portInfo,
        std::shared_ptr<Lv2PortViewFactory> customization = nullptr);


    Lv2cStereoValueElement::ptr CreateStereoPortView(
        const std::string&label,
        Lv2cBindingProperty<double> &leftValue,
        Lv2cBindingProperty<double> &rightValue,
        const Lv2PortInfo&portInfo,
        std::shared_ptr<Lv2PortViewFactory> customization = nullptr);

}





