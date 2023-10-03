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
#include "lvtk/LvtkBindingProperty.hpp"
#include "lvtk_ui/Lv2PluginInfo.hpp"

#include <string>
#include <memory>
#include "Lv2Units.hpp"

namespace lvtk::ui {
    class Lv2PortInfo;

    enum Lv2PortViewType {
        Invalid,
        Dial,
        Dropdown,
        Toggle,
        OnOff,
        VuMeter,
        StereoVuMeter,
        StereoVuMeterRight,
        LED,
        StatusOutputMessage,
        Other
    };
    class Lv2PortViewController {
    public:
        using self = Lv2PortViewController;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        
        Lv2PortViewController();
        Lv2PortViewController(const Lv2PortInfo &portInfo);
        virtual ~Lv2PortViewController() { }


        Lv2PortViewType GetViewType() const;

        LvtkBindingProperty<double> DialValueProperty;
        Lv2PortViewController&DialValue(double value);
        double DialValue() const;

        LvtkBindingProperty<double> PortValueProperty;
        Lv2PortViewController&PortValue(double value);
        double PortValue() const;

        LvtkBindingProperty<double> RightPortValueProperty;
        Lv2PortViewController&RightPortValue(double value);
        double RightPortValue() const;


        LvtkBindingProperty<int64_t> DropdownValueProperty;
        Lv2PortViewController&DropdownValue(int64_t value);
        int64_t DropdownValue() const;


        LvtkBindingProperty<bool> IsDraggingProperty;
        bool IsDragging() const;
        Lv2PortViewController&IsDragging(bool value);

        bool IsInputControl() const;

        bool IsEnumeration() const;


        bool IsInteger() const;

        bool IsToggle() const;
        
        LvtkBindingProperty<std::string> CaptionProperty;
        Lv2PortViewController&Caption(const std::string&  value);
        const std::string& Caption() const;

        LvtkBindingProperty<std::string> DisplayValueProperty;
        Lv2PortViewController &DisplayValue(const std::string& value);
        const std::string& DisplayValue() const;

        Lv2Units Units() const;

        double MaxValue() const;

        double MinValue() const;

        double DefaultValue() const;

        bool Logarithmic() const;

        const Lv2PortInfo&PortInfo() const;
        Lv2PortInfo&PortInfo();
    private:
        std::string GetDisplayString(double value) const;
        std::string AutoRangeValue(double value, const std::string suffix) const;
        std::string GetIntegerDisplayString(double value) const;

        Lv2PortViewType CalculateViewType();

        Lv2PortViewType viewType = Lv2PortViewType::Invalid;
        void Init();
        Lv2PortInfo portInfo;
        void OnIsDraggingChanged(const bool&value);
        void OnDialValueChanged(double value);
        void OnPortValueChanged(double value);
        void OnDropdownValueChanged(int64_t value);
        void OnRightPortValueChanged(double value);
        void UpdateDisplayValue(float value);

        observer_handle_t dialValueChangedHandle;
        observer_handle_t portValueChangedHandle;
        observer_handle_t dialDraggingHandle;
        observer_handle_t dropdownValueChangedHandle;

        double dragPortValue = 0;

    };



}