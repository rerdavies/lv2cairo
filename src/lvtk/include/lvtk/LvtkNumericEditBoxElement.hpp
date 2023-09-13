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

#include "LvtkEditBoxElement.hpp"
#include "LvtkBindingProperty.hpp"
#include <optional>

namespace lvtk {

    enum LvtkValueType {
        Int8,
        Uint8,
        Int16,
        Uint16,
        Int32,
        Uint32,
        Int64,
        UInt64,
        Uint64,
        Float,
        Double
    };

    class LvtkNumericEditBoxElement: public LvtkEditBoxElement {
    public:
        using super=LvtkEditBoxElement;
        using self=LvtkNumericEditBoxElement;
        using ptr=std::shared_ptr<LvtkNumericEditBoxElement>;
        static ptr Create() { return std::make_shared<LvtkNumericEditBoxElement>(); }

        LvtkNumericEditBoxElement();



        LvtkBindingProperty<double> ValueProperty { 0};
        self& Value(double value) { ValueProperty.set(value); return *this; }
        double Value() const { return ValueProperty.get(); }

        BINDING_PROPERTY(ValueType,LvtkValueType,LvtkValueType::Double)
        BINDING_PROPERTY(MinValue,std::optional<double>,std::optional<double>())
        BINDING_PROPERTY(MaxValue,std::optional<double>,std::optional<double>())
        BINDING_PROPERTY_REF(DisplayValue,std::string,"")

    protected:
        virtual void OnValueTypeChanged(LvtkValueType value);
        virtual void OnRangeChanged(const std::optional<double> &value);
        virtual void OnValueChanged(double value);
        virtual void OnTextChanged(const std::string &value) override;

        virtual bool TextToValue(const std::string&value, double *pResult);
        virtual void UpdateErrorState(const std::string&value);
        virtual void OnMount() override;


    protected:
        virtual bool OnKeyDown(const LvtkKeyboardEventArgs&event) override;
        virtual bool OnFocus(const LvtkFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const LvtkFocusEventArgs &eventArgs) override;


        observer_handle_t displayValueObserverHandle;
    private: 
        void CommitValue();
        void CancelValue();
        std::string lastGoodDisplayValue;
        double lastGoodValue;
        bool displayValueChanging = false;
    public: // public for testing.
        static std::string NiceEditText(double value,LvtkValueType numericType);
    };
}