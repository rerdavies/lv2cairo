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

#include "lvtk/LvtkNumericEditBoxElement.hpp"
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <limits>
#include <cmath>
#include <iomanip>

using namespace lvtk;

LvtkNumericEditBoxElement::LvtkNumericEditBoxElement()
{
    MinValueProperty.SetElement(this, &LvtkNumericEditBoxElement::OnRangeChanged);
    MaxValueProperty.SetElement(this, &LvtkNumericEditBoxElement::OnRangeChanged);
    ValueProperty.SetElement(this, &LvtkNumericEditBoxElement::OnValueChanged);
    ValueTypeProperty.SetElement(this, &LvtkNumericEditBoxElement::OnValueTypeChanged);
    OnValueChanged(Value());
    SelectAll();
}

void LvtkNumericEditBoxElement::OnRangeChanged(const std::optional<double> &value)
{
    UpdateErrorState(Text());
}

void LvtkNumericEditBoxElement::OnValueChanged(double value)
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

void LvtkNumericEditBoxElement::OnValueTypeChanged(LvtkValueType value)
{

    if (!displayValueChanging)
    {
        std::string v = NiceEditText(Value(), ValueType());
        DisplayValue(v);
        Text(v);
        SelectAll();
    }
}

void LvtkNumericEditBoxElement::OnTextChanged(const std::string &value)
{
    super::OnTextChanged(value);
    UpdateErrorState(value);
}

void LvtkNumericEditBoxElement::UpdateErrorState(const std::string &value)
{
    double result;
    bool errorState = !TextToValue(value, &result);
    if (Text().length() == 0)
    {
        errorState = false;
    }
    this->ShowError(errorState);
}
bool LvtkNumericEditBoxElement::TextToValue(const std::string &value, double *pResult)
{
    *pResult = 0;
    std::stringstream s(value);
    double numericValue = 0;

#define VALUE_CASE(ENUM_VALUE, TYPE) \
    case LvtkValueType::ENUM_VALUE: \
    {                                \
        TYPE v = 0;                  \
        s >> v;                      \
        numericValue = v;            \
    }                                \
    break;

#define VALUE_CASE_R(ENUM_VALUE, TYPE)                                                    \
    case LvtkValueType::ENUM_VALUE:                                                      \
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

    case LvtkValueType::Float:
    {
        float v = 0;
        s >> v;
        numericValue = v;
        break;
    }
    case LvtkValueType::Double:
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

std::string LvtkNumericEditBoxElement::NiceEditText(double value, LvtkValueType numericType)
{
    std::stringstream s;
    bool isInteger = numericType != LvtkValueType::Float && numericType != LvtkValueType::Double;
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

bool LvtkNumericEditBoxElement::OnFocus(const LvtkFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    return false;
}
bool LvtkNumericEditBoxElement::OnLostFocus(const LvtkFocusEventArgs &eventArgs)
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

void LvtkNumericEditBoxElement::CommitValue()
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

void LvtkNumericEditBoxElement::CancelValue()
{
    displayValueChanging = true;
    Text(lastGoodDisplayValue);
    Value(lastGoodValue);
    displayValueChanging = false;
    SelectAll();
}

bool LvtkNumericEditBoxElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
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

void LvtkNumericEditBoxElement::OnMount()
{
    super::OnMount(); // resets classes.
    this->AddClass(Theme().numericEditBoxStyle);
}