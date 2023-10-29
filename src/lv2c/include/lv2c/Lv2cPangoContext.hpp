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

#include <set>
#include <string>

typedef struct _PangoContext PangoContext;
typedef struct _PangoFontMap PangoFontMap;
typedef struct _PangoFontDescription PangoFontDescription;
namespace lvtk
{
    class Lv2cStyle;

    const std::set<std::string> &FontFamilies();

    class Lv2cPangoContext
    {
    public:
        Lv2cPangoContext();
        PangoContext *get() { return pangoContext; }

        const std::set<std::string> &FontFamilies();

        /// @brief Get an installed font from a list of fonts.
        /// @param fontFamily A css-style list of font families.
        /// @return The first font in the list of requested fonts that is currently installed.
        const std::string GetFontFamily(const std::string&fontFamilies) const;
        PangoFontDescription*GetFontDescription(Lv2cStyle&style) const;
    private:
        PangoContext*pangoContext = nullptr;
        PangoFontMap *fontmap = nullptr;
        std::set<std::string> fontFamilies;
    };

    extern Lv2cPangoContext gPangoContext;

} // namespace
