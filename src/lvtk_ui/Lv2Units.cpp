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


#include "lvtk_ui/Lv2Units.hpp"
#include "lvtk_ui/Lv2Exception.hpp"
#include <algorithm>
#include <map>

#include  "lv2/lv2plug.in/ns/extensions/units/units.h"


using namespace lvtk::ui;

Lv2Units UriToUnits(const std::string &uri);


#define CASE(name) \
   { Lv2Units::name, #name },


std::map<Lv2Units,std::string> unitsToStringMap = 
{
    //LV2_UNITS__conversion
    //LV2_UNITS__name
        CASE(none)
        CASE(unknown)
        CASE(bar)
        CASE(beat)
        CASE(bpm)
        CASE(cent)
        CASE(cm)
        CASE(coef)
        CASE(db)
        CASE(degree)
        CASE(frame)
        CASE(hz)
        CASE(inch)
        CASE(khz)
        CASE(km)
        CASE(m)
        CASE(mhz)
        CASE(midiNote)
        CASE(mile)
        CASE(min)
        CASE(mm)
        CASE(ms)
        CASE(oct)
        CASE(s)
        CASE(pc)
        CASE(semitone12TET)

};

static const std::string emptyString;
const std::string& lvtk::ui::UnitsToString(Lv2Units units)
{
    if (unitsToStringMap.find(units) != unitsToStringMap.end())
    {
        return unitsToStringMap[units];
    } else {
        return emptyString;
    }
}
#undef CASE

#define CASE(name) \
     { #name, Lv2Units::name},

std::map<std::string,Lv2Units> unitMap = {
        CASE(none)
        CASE(unknown)
        CASE(bar)
        CASE(beat)
        CASE(bpm)
        CASE(cent)
        CASE(cm)
        CASE(coef)
        CASE(db)
        CASE(degree)
        CASE(frame)
        CASE(hz)
        CASE(inch)
        CASE(khz)
        CASE(km)
        CASE(m)
        CASE(mhz)
        CASE(midiNote)
        CASE(mile)
        CASE(min)
        CASE(mm)
        CASE(ms)
        CASE(oct)
        CASE(s)
        CASE(pc)
        CASE(semitone12TET)
};
#undef CASE

#define CASE(name) \
     { LV2_UNITS__##name, Lv2Units::name},



std::map<std::string,Lv2Units> uriToUnitsMap = {
        { "", Lv2Units::none,},
        CASE(bar)
        CASE(beat)
        CASE(bpm)
        CASE(cent)
        CASE(cm)
        CASE(coef)
        CASE(db)
        CASE(degree)
        CASE(frame)
        CASE(hz)
        CASE(inch)
        CASE(khz)
        CASE(km)
        CASE(m)
        CASE(mhz)
        CASE(midiNote)
        CASE(mile)
        CASE(min)
        CASE(mm)
        CASE(ms)
        CASE(oct)
        CASE(s)
        CASE(pc)
        CASE(semitone12TET)
};
#undef CASE


Lv2Units lvtk::ui::StringToUnits(const std::string &text)
{
    if (unitMap.find(text) != unitMap.end()) {
        return unitMap[text];
    }
    return Lv2Units::none;
}

Lv2Units lvtk::ui::UriToUnits(const std::string &text)
{
    if (text.length() == 0) return Lv2Units::none;
    if (uriToUnitsMap.contains(text))
    {
        auto result = uriToUnitsMap[text];
        return result;
    }
    return Lv2Units::unknown;
}



#ifdef LV2_JSON
class UnitsEnumConverter: public json_enum_converter<Lv2Units> {
public:
    virtual Lv2Units fromString(const std::string&value) const
    {
        return StringToUnits(value);
    }
    virtual const std::string& toString(Lv2Units value) const
    {
        return UnitsToString(value);
    }
    

} g_units_converter;


json_enum_converter<Lv2Units> *lvtk::ui::get_units_enum_converter()
{
    return &g_units_converter;
}
#endif
