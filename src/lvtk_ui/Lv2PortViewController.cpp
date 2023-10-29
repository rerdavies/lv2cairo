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

#include "lvtk_ui/Lv2PortViewController.hpp"
#include "lvtk_ui/PiPedalUiDefs.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <lv2/port-groups/port-groups.h>
using namespace std;

namespace lvtk::ui
{

    std::string Lv2PortViewController::GetDisplayString(double value) const
    {
        double v = value;
        if (std::isinf(v))
        {
            return "INF";
        }
        if (std::isnan(v))
        {
            return "NaN";
        }
        std::stringstream s;
        if (IsInteger())
        {
            s << (int64_t)v;
            return s.str();
        }
        if (std::abs(v) < 0.001)
        {
            v = 0;
        }
        if (v < 0)
        {
            s << '-';
            v = -v;
        }
        if (v >= 100)
        {
            s << (int64_t)(std::round(v));
        }
        else if (v >= 9.95)
        {
            v = ((int64_t)std::round(v * 10)) / 10.0;
            int64_t iValue = (int64_t)std::floor(v);
            s << iValue;
            s << '.';
            int64_t iFrac = (int64_t)std::round((v - iValue) * 10);
            if (iFrac >= 10)
                iFrac = 9;
            s << iFrac;
        }
        else
        {
            v = ((int64_t)std::round(v * 100)) / 100.0;

            int64_t iValue = (int64_t)std::floor(v);
            s << iValue;
            s << '.';
            uint64_t iFrac = (uint64_t)std::round((v - iValue) * 100);
            if (iFrac >= 100)
                iFrac = 99;
            s << (char)((iFrac / 10) + '0');
            s << (char)((iFrac % 10) + '0');
        }
        return s.str();
    }
    std::string Lv2PortViewController::GetIntegerDisplayString(double value) const
    {
        // prefer integer values.
        double frac = value - std::round(value);
        if (frac < 1E-5)
        {
            std::stringstream s;
            s << ((int64_t)std::round(value));
            return s.str();
        }
        else
        {
            return GetDisplayString(value);
        }
    }
    std::string Lv2PortViewController::AutoRangeValue(double value, const std::string suffix) const
    {
        std::stringstream s;
        double absValue = std::abs(value);
        if (IsInteger())
        {
            s << (int64_t)value;
        }
        else if (absValue < 1)
        {
            if (absValue < 1E-12)
            {
                s << GetDisplayString(0);
            }
            else if (absValue < 1E-9)
            {
                s << GetDisplayString(value * 1E12) << "p";
            }
            else if (absValue < 1E-6)
            {
                s << GetDisplayString(value * 1E9) << "n";
            }
            else if (absValue < 1E-3)
            {
                s << GetDisplayString(value * 1E6) << "µ";
            }
            else // if (absValue < 1)
            {
                s << GetDisplayString(value * 1E3) << "m";
            }
        }
        else
        {

            if (absValue >= 1E21)
            {
                s << GetDisplayString(value * 1E-21) << "Z";
            }
            else if (absValue >= 1E18)
            {
                s << GetDisplayString(value * 1E-18) << "E";
            }
            else if (absValue >= 1E15)
            {
                s << GetDisplayString(value * 1E-15) << "P";
            }
            else if (absValue >= 1E12)
            {
                s << GetDisplayString(value * 1E-12) << "T";
            }
            else if (absValue >= 1E9)
            {
                s << GetDisplayString(value * 1E-9) << "G";
            }
            else if (absValue >= 1E6)
            {
                s << GetDisplayString(value * 1E-6) << "M";
            }
            else if (absValue >= 1E3)
            {
                s << GetDisplayString(value * 1E-3) << "k";
            }
            else
            {
                s << GetDisplayString(value);
            }
        }
        s << suffix;
        return s.str();
    }

    Lv2PortViewController &Lv2PortViewController::DialValue(double value)
    {
        DialValueProperty.set(value);
        return *this;
    }

    double Lv2PortViewController::DialValue() const
    {
        return DialValueProperty.get();
    }

    Lv2PortViewController &Lv2PortViewController::Caption(const std::string &value)
    {
        CaptionProperty.set(value);
        portInfo.name(value);
        return *this;
    }

    const std::string &Lv2PortViewController::Caption() const
    {
        return CaptionProperty.get();
    }

    Lv2PortViewController &Lv2PortViewController::DisplayValue(const std::string &value)
    {
        DisplayValueProperty.set(value);
        return *this;
    }
    const std::string &Lv2PortViewController::DisplayValue() const
    {
        return DisplayValueProperty.get();
    }

    static bool FloatEqual(double v1, double v2)
    {
        return std::abs(v1 - v2) < 1E-7;
    }

    void Lv2PortViewController::OnRightPortValueChanged(double value)
    {

    }

    void Lv2PortViewController::OnDropdownValueChanged(int64_t value)
    {

        if (viewType == Lv2PortViewType::Dropdown )
        {
            if (value >= 0 && value < (int64_t)(this->portInfo.scale_points().size()) )
            {
                this->PortValue(this->portInfo.scale_points()[value].value());
            }
        }
    }


    void Lv2PortViewController::OnPortValueChanged(double value_)
    {
        double value = value_;
        if (IsDragging())
        {
            value = dragPortValue;
        }
        double dialValue;
        if (Logarithmic())
        {
            // beware of invalid state during setup.
            // don't update dependents if the value is bad.
            if (value <= 0)
                return;
            double logMin = std::log(MinValue());
            double logMax = std::log(MaxValue());
            double logValue = std::log(value);

            dialValue = (logValue - logMin) / (logMax - logMin);
            if (std::isnan(dialValue))
                return; // break circular updates now!
        }
        else
        {
            dialValue = (value - MinValue()) / (MaxValue() - MinValue());
        }
        if (!FloatEqual(dialValue, DialValueProperty.get()))
        {
            DialValueProperty.set(dialValue);
        }
        auto viewType = GetViewType();
        if (viewType == Lv2PortViewType::Dropdown || viewType == Lv2PortViewType::StatusOutputMessage)
        {
            int64_t id = -1;
            int64_t dropdownId = 0;
            for (auto&scale_point : this->portInfo.scale_points()) {
                if (value == scale_point.value())
                {
                    id = dropdownId;
                    break;
                }
                ++dropdownId;
            }
            this->DropdownValue(id);
        }
        UpdateDisplayValue(value);
    }

    void Lv2PortViewController::OnDialValueChanged(double value)
    {
        // Don't allow integer Lamp control to send
        if (!IsInputControl())
        {
            return;
        }
        if (IsInteger())
        {

            double t = value * (MaxValue() - MinValue()) + MinValue();
            if (!FloatEqual(t, dragPortValue))
            {
                dragPortValue = t;
                double iValue = (int64_t)std::round(t);
                PortValue(iValue);
            }
        }
        else if (Logarithmic())
        {
            double logMinValue = std::log(MinValue());
            double logMaxValue = std::log(MaxValue());
            double portValue = std::exp((logMaxValue - logMinValue) * value + logMinValue);

            dragPortValue = portValue;
            PortValue(portValue);
            // circular updates are broken at OnPortValueChanged.
        }
        else
        {
            double portValue = value * (MaxValue() - MinValue()) + MinValue();
            if (!FloatEqual(portValue, PortValue()))
            {
                dragPortValue = portValue;
                PortValue(portValue);
            }
        }
    }

    static std::string semitoneNames[] = {
        "C", "C♯", "D", "Eb", "E", "F", "F♯", "G", "Ab", "A", "Bb", "B"};
    static std::string Semitone(double value_)
    {
        int32_t value = (int32_t)value_;
        if (value < 0 || value >= 12)
        {
            std::stringstream s;
            s << value;
            return s.str();
        }
        return semitoneNames[value];
    }

    std::string MidiNote(double value_)
    {
        int32_t value = (int32_t)value_;
        if (value < 0)
            return "";
        int32_t octave = value / 12;
        int32_t semitone = value - octave * 12;

        std::stringstream s;
        s << (octave - 1) << Semitone(semitone);
        return s.str();
    }

    void Lv2PortViewController::UpdateDisplayValue(float value)
    {
        if (IsInteger())
        {
            value = (int64_t)std::round(value);
        }
        std::string text;
        bool hasScalePoint = false;
        for (auto &scalePoint : portInfo.scale_points())
        {
            if (scalePoint.value() == value)
            {
                text = scalePoint.label();
                hasScalePoint = true;
                break;
            }
        }

        if (!hasScalePoint)
        {
            switch (Units())
            {
            case Lv2Units::none:
            case Lv2Units::unknown:
                text = GetDisplayString(value);
                break;
            case Lv2Units::bar:
            case Lv2Units::beat:
            case Lv2Units::bpm: // "90", not "90.0". GetIntegerDisplayString will fall back if bmp is not an integer, and produce "84.2".
                text = GetIntegerDisplayString(value);
                break;
            case Lv2Units::cent:
                if (value > 0)
                {
                    text = "+" + GetDisplayString(value);
                }
                else
                {
                    text = GetDisplayString(value);
                }
                break;
            case Lv2Units::cm:
                text = GetDisplayString(value) + "cm";
                break;
            case Lv2Units::db:
                text = GetDisplayString(value) + "dB";
                break;
            case Lv2Units::hz:
                text = AutoRangeValue(value, "hz");
                break;
            case Lv2Units::khz:
                text = AutoRangeValue(value * 1000, "hz");
                break;
            case Lv2Units::km:
                text = AutoRangeValue(value * 1000, "hz");
                break;
            case Lv2Units::m:
                text = AutoRangeValue(value, "m");
                break;
            case Lv2Units::mhz:
                text = AutoRangeValue(value * 1000 * 100, "hz");
                break;
            case Lv2Units::midiNote:
                text = MidiNote(value);
                break;
            case Lv2Units::min:
                text = GetDisplayString(value) + "min";
                break;

            case Lv2Units::ms:
                text = AutoRangeValue(value * 0.001, "s");
                break;

            case Lv2Units::pc:
                text = GetDisplayString(value) + "%";
                break;
            case Lv2Units::s:
                text = AutoRangeValue(value, "s");
                break;

            case Lv2Units::semitone12TET:
                text = GetIntegerDisplayString(value) + "semi";
                break;
            case Lv2Units::custom:
                text = GetDisplayString(value);
                break;
            case Lv2Units::degree:
                text = GetDisplayString(value) + "°";
                break;
            case Lv2Units::coef:
                text = GetDisplayString(value) + "x";
                break;
            case Lv2Units::frame:
                text = GetIntegerDisplayString(value);
                break;
            case Lv2Units::inch:
                text = GetDisplayString(value) + "″";
                break;
            case Lv2Units::mile:
                text = GetDisplayString(value) + "mi";
                break;
            case Lv2Units::mm:
                text = AutoRangeValue(value * 0.001, "m");
                break;
            case Lv2Units::oct:
                text = GetIntegerDisplayString(value);
                break;
            default:
                text = GetDisplayString(value);
                break;
            }
        }
        this->DisplayValue(text);
    }


    Lv2PortViewController::Lv2PortViewController(const Lv2PortInfo &portInfo)
        : portInfo(portInfo)
    {
        Init();
    }
    void Lv2PortViewController::Init()
    {
        viewType = CalculateViewType();
        CaptionProperty.set(portInfo.name());

        OnDialValueChanged(DialValue());
        dialValueChangedHandle = this->DialValueProperty.addObserver(
            [this](double value)
            {
                OnDialValueChanged(value);
            });
        portValueChangedHandle = this->PortValueProperty.addObserver(
            [this](double value)
            {
                OnPortValueChanged(value);
            });
        dropdownValueChangedHandle = this->DropdownValueProperty.addObserver(
            [this](int64_t value)
            {
                OnDropdownValueChanged(value);
            }
        );
        dialDraggingHandle = this->IsDraggingProperty.addObserver(
            [this](bool value)
            {
                OnIsDraggingChanged(value);
            });
        OnPortValueChanged(PortValue());
    }

    Lv2PortViewController::Lv2PortViewController()
    {
        Init();
    }

    double Lv2PortViewController::MaxValue() const
    {
        return portInfo.max_value();
    }

    double Lv2PortViewController::MinValue() const
    {
        return portInfo.min_value();
    }

    double Lv2PortViewController::DefaultValue() const
    {
        return portInfo.default_value();
    }

    bool Lv2PortViewController::Logarithmic() const
    {
        return portInfo.is_logarithmic();
    }

    Lv2PortViewController &Lv2PortViewController::PortValue(double value)
    {
        PortValueProperty.set(value);
        OnPortValueChanged(PortValue());
        return *this;
    }

    double Lv2PortViewController::PortValue() const
    {
        return PortValueProperty.get();
    }
    Lv2PortViewController &Lv2PortViewController::RightPortValue(double value)
    {
        RightPortValueProperty.set(value);
        OnRightPortValueChanged(RightPortValue());
        return *this;
    }

    double Lv2PortViewController::RightPortValue() const
    {
        return RightPortValueProperty.get();
    }

    Lv2PortViewController &Lv2PortViewController::DropdownValue(int64_t value)
    {
        DropdownValueProperty.set(value); return *this;
    }
    int64_t Lv2PortViewController::DropdownValue() const
    {
        return DropdownValueProperty.get();
    }

    Lv2Units Lv2PortViewController::Units() const
    {
        return portInfo.units();
    }
    

    bool Lv2PortViewController::IsInteger() const
    {
        return portInfo.integer_property();
    }
    const Lv2PortInfo &Lv2PortViewController::PortInfo() const
    {
        return portInfo;
    }
    Lv2PortInfo &Lv2PortViewController::PortInfo()
    {
        return portInfo;
    }

    void Lv2PortViewController::OnIsDraggingChanged(const bool &isDragging)
    {
        if (isDragging)
        {
            // we'll be using this value going forward.
            dragPortValue = PortValue();
        }
        else
        {
            // revert to the most recent port value from the host.
            // if there are updates on the wire, we'll get them sooner or later.
            this->OnPortValueChanged(PortValue());
        }
    }

    bool Lv2PortViewController::IsInputControl() const
    {
        return this->portInfo.is_input();
    }
    
    bool Lv2PortViewController::IsEnumeration() const
    {
        return this->portInfo.enumeration_property();
    }

    bool Lv2PortViewController::IsToggle() const
    {
        return this->portInfo.toggled_property();
    }

    bool Lv2PortViewController::IsDragging() const
    {
        return IsDraggingProperty.get();
    }
    Lv2PortViewController &Lv2PortViewController::IsDragging(bool value)
    {
        IsDraggingProperty.set(value);
        return *this;
    }

    Lv2PortViewType Lv2PortViewController::GetViewType() const {
        return viewType;
    }
    Lv2PortViewType Lv2PortViewController::CalculateViewType()
    {
        if (this->IsInputControl())
        {
            if (IsEnumeration())
            {
                return Lv2PortViewType::Dropdown;
            }
            if (IsToggle())

            {
                if (portInfo.scale_points().size() == 2)
                {
                    return Lv2PortViewType::Toggle;
                }
                return Lv2PortViewType::OnOff;
            }
            return Lv2PortViewType::Dial;
        }
        else
        {

            if (portInfo.units() == Lv2Units::midiNote  // use this
                || portInfo.designation() == PIPEDAL_UI__tunerFrequency  // deprecated (not this)
                )
            {
                return Lv2PortViewType::Tuner;
            }
            if (IsEnumeration())
            {
                return Lv2PortViewType::StatusOutputMessage;
            }
            if (portInfo.designation() == LV2_PORT_GROUPS__left)
            {
                return Lv2PortViewType::StereoVuMeter;
            }
            if (portInfo.designation() == LV2_PORT_GROUPS__right)
            {
                return Lv2PortViewType::StereoVuMeterRight;
            }
            if (portInfo.max_value() == 1.0 && portInfo.min_value() == 0.0 && portInfo.integer_property())
            {
                return Lv2PortViewType::LED;
            }
            if (IsToggle() && portInfo.min_value() == 0.0)
            {
                return Lv2PortViewType::LED;
            }
            return Lv2PortViewType::VuMeter;
        }
    }

} // namespace
