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

#include "lv2c/Lv2cNumericEditBoxElement.hpp"
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <limits>
#include <cmath>
#include <iomanip>

using namespace lvtk;

Lv2cNumericEditBoxElement::Lv2cNumericEditBoxElement()
{
    MinValueProperty.SetElement(this, &Lv2cNumericEditBoxElement::OnRangeChanged);
    MaxValueProperty.SetElement(this, &Lv2cNumericEditBoxElement::OnRangeChanged);
    ValueProperty.SetElement(this, &Lv2cNumericEditBoxElement::OnValueChanged);
    ValueTypeProperty.SetElement(this, &Lv2cNumericEditBoxElement::OnValueTypeChanged);
    OnValueChanged(Value());
    SelectAll();
}

void Lv2cNumericEditBoxElement::OnRangeChanged(const std::optional<double> &value)
{
    UpdateErrorState(Text());
}

void Lv2cNumericEditBoxElement::OnValueChanged(double value)
{
    if (!displayValueChanging)
    {
        std::string v = NiceEditText(value, ValueType());
        DisplayValue(v);
        Text(v);
        SelectAll();
        lastGoodValue = value;
        lastGoodDisplayValue = v;
    }
}

void Lv2cNumericEditBoxElement::OnValueTypeChanged(Lv2cValueType value)
{

    if (!displayValueChanging)
    {
        std::string v = NiceEditText(Value(), ValueType());
        DisplayValue(v);
        Text(v);
        SelectAll();
    }
}

void Lv2cNumericEditBoxElement::OnTextChanged(const std::string &value)
{
    super::OnTextChanged(value);
    UpdateErrorState(value);
}

void Lv2cNumericEditBoxElement::UpdateErrorState(const std::string &value)
{
    double result;
    bool errorState = !TextToValue(value, &result);
    if (Text().length() == 0)
    {
        errorState = false;
    }
    this->ShowError(errorState);
}
bool Lv2cNumericEditBoxElement::TextToValue(const std::string &value, double *pResult)
{
    *pResult = 0;
    std::stringstream s(value);
    double numericValue = 0;

#define VALUE_CASE(ENUM_VALUE, TYPE) \
    case Lv2cValueType::ENUM_VALUE: \
    {                                \
        TYPE v = 0;                  \
        s >> v;                      \
        numericValue = v;            \
    }                                \
    break;

#define VALUE_CASE_R(ENUM_VALUE, TYPE)                                                    \
    case Lv2cValueType::ENUM_VALUE:                                                      \
    {                                                                                     \
        int64_t v = 0;                                                                    \
        s >> v;                                                                           \
        if (v > std::numeric_limits<TYPE>::max() || v < std::numeric_limits<TYPE>::min()) \
            valid = false;                                                                \
        numericValue = v;                                                                 \
    }                                                                                     \
    break;

    bool valid = true;
    switch (ValueType())
    {
        VALUE_CASE_R(Int8, int8_t)
        VALUE_CASE_R(Uint8, uint8_t)
        VALUE_CASE(Int16, int16_t)
        VALUE_CASE(Uint16, uint16_t)
        VALUE_CASE(Int32, int32_t)
        VALUE_CASE(Uint32, uint32_t)
        VALUE_CASE(Int64, int64_t)
        VALUE_CASE(UInt64, uint64_t)

    case Lv2cValueType::Float:
    {
        float v = 0;
        s >> v;
        numericValue = v;
        break;
    }
    case Lv2cValueType::Double:
    {
        double v = 0;
        s >> v;
        numericValue = v;
        break;
    }
    default:
        valid = false;
        break;
    }

    if (s.fail())
    {
        valid = false;
    }

    while (s.peek() == ' ')
    {
        s.get();
    }
    if (s.get() != EOF)
    {
        valid = false;
    }
    if (valid && MinValue().has_value())
    {
        if ((float)numericValue < (float)(MinValue().value()))
        {
            valid = false;
        }
    }
    if (valid && MaxValue().has_value())
    {
        if ((float)numericValue > (float)(MaxValue().value()))
        {
            valid = false;
        }
    }
    if (valid)
    {
        *pResult = numericValue;
    }
    return valid;
}

std::string Lv2cNumericEditBoxElement::NiceEditText(double value, Lv2cValueType numericType)
{
    std::stringstream s;
    bool isInteger = numericType != Lv2cValueType::Float && numericType != Lv2cValueType::Double;
    if (isInteger)
    {
        s << (int64_t)value;
        return s.str();
    }
    if (value < 0)
    {
        s << "-";
        value = -value;
    }
    if (value < 1E-9)
    {
        return "0.000";
    }
    if (std::round(value) >= 1E6 || value < 1E-4)
    {
        s << std::scientific;
        s << std::setprecision(4)
          << value;
        return s.str();
    }

    uint64_t iValue;
    if ((iValue = (uint64_t)std::round(value)) >= 1000)
    {
        s << iValue;
        return s.str();
    }
    int64_t exp = 0;
    while ((iValue = std::round(value * std::pow(10, exp))) < 1000)
    {
        ++exp;
        if (exp > 45)
            return "NaN";
    }
    uint64_t digits = 0;
    exp = 4 - exp;
    if (exp <= 0)
    {
        s << "0.";
        for (int64_t i = 0; i < -exp; ++i)
        {
            s << "0";
        }
    }
    while (digits < 4)
    {
        char c = '0' + (iValue / 1000) % 10;
        s << c;
        iValue = (iValue * 10) % 10000;
        ++digits;
        --exp;
        if (exp == 0 && digits != 4)
        {
            s << '.';
        }
    }
    return s.str();
}

bool Lv2cNumericEditBoxElement::OnFocus(const Lv2cFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    return false;
}
bool Lv2cNumericEditBoxElement::OnLostFocus(const Lv2cFocusEventArgs &eventArgs)
{
    super::OnLostFocus(eventArgs);
    if (!ShowError())
    {
        CommitValue();
    }
    else
    {
        CancelValue();
    }
    return false;
}

void Lv2cNumericEditBoxElement::CommitValue()
{
    double value;
    std::string text = Text();
    if (text.length() == 0)
    {
        text = "0";
        if (MinValue().has_value() && MinValue().value() > 0)
        {
            std::stringstream s;
            s << MinValue().value();
            text = s.str();
        }
    }
    if (TextToValue(text, &value))
    {
        displayValueChanging = true;
        lastGoodDisplayValue = Text();
        Value(value);
        // DisplayValue(NiceEditText(value,ValueType()));
        lastGoodValue = Value();
        displayValueChanging = false;
    }
}

void Lv2cNumericEditBoxElement::CancelValue()
{
    displayValueChanging = true;
    Text(lastGoodDisplayValue);
    Value(lastGoodValue);
    displayValueChanging = false;
    SelectAll();
}

bool Lv2cNumericEditBoxElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
{
    if (event.keysymValid)
    {
        if (event.modifierState == ModifierState::Empty)
        {
            switch (event.keysym)
            {
            case XK_KP_Enter:
            case XK_Return:
                CommitValue();
                return false;
            case XK_Escape:
            case XK_Cancel:
                CancelValue();
                return false;
            default:
                break;
            }
        }
    }
    return super::OnKeyDown(event);
}

void Lv2cNumericEditBoxElement::OnMount()
{
    super::OnMount(); // resets classes.
    this->AddClass(Theme().numericEditBoxStyle);
}