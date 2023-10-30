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

#include "lv2c/Lv2cPangoContext.hpp"
#include "lv2c/Lv2cStyle.hpp"
#include "pango/pangocairo.h"
#include <vector>
#include <sstream>

using namespace lv2c;

Lv2cPangoContext lv2c::gPangoContext;


const std::set<std::string> &lv2c::FontFamilies()
{
    return gPangoContext.FontFamilies();
}

Lv2cPangoContext::Lv2cPangoContext()
{
    fontmap = pango_cairo_font_map_get_default();
}

const std::set<std::string> &Lv2cPangoContext::FontFamilies()
{
    if (this->fontFamilies.size() != 0)
    {
        return this->fontFamilies;
    }
    PangoFontFamily **families;
    int nFamilies;

    PangoFontMap *fontmap = pango_cairo_font_map_get_default();

    pango_font_map_list_families(fontmap, &families, &nFamilies);
    for (int i = 0; i < nFamilies; ++i)
    {
        PangoFontFamily *family = families[i];
        const char *name = pango_font_family_get_name(family);
        fontFamilies.insert(name);
    }
    return fontFamilies;
}

static std::vector<std::string> splitFamilies(const std::string &text, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream s(text);
    std::string line;
    while (std::getline(s, line, delimiter))
    {
        size_t start = 0;
        while (start < line.length() && line[start] == ' ')
            ++start;
        size_t end;
        for (end = line.length(); end > start && line[end - 1] == ' '; --end)
        {
        }
        std::string t = line.substr(start, end - start);
        if (t.starts_with('\'') && t.ends_with('\''))
        {
            t = std::string(t.begin()+1,t.end()-1);
        }
        if (t.length() != 0)
        {
            result.push_back(t);
        }
    }
    return result;
}

const std::string Lv2cPangoContext::GetFontFamily(const std::string&fontFamily) const
{
    
    std::vector<std::string> familyNames = splitFamilies(fontFamily, ',');

    const std::set<std::string> &installedFamilies = gPangoContext.FontFamilies();
    std::string result;
    for (const std::string &familyName : familyNames)
    {
        if (installedFamilies.contains(familyName))
        {
            result = familyName;
            break;
        }
    }
    if (result.length() == 0)
    {
        result = "Serif";
    }
    return result;

}


PangoFontDescription*Lv2cPangoContext::GetFontDescription(Lv2cStyle&style) const
{
    std::string fontFamily = style.FontFamily();
    std::string installedFont = GetFontFamily(fontFamily);


    PangoFontDescription *desc = pango_font_description_new();

    pango_font_description_set_family(desc, installedFont.c_str());
    double fontSize = style.FontSize().PixelValue();
    if (fontSize == 0)
    {
        fontSize = 12;
    }
    pango_font_description_set_size(
        desc,
        (gint)(fontSize * 72.0 / 96 * PANGO_SCALE));

    auto variant = style.FontVariant();
    if (variant.has_value())
    {
        // cast enum class Lv2cFontVariant to enum PangoVariant!
        PangoVariant pangoVariant = (PangoVariant)(int)(variant.value());
        pango_font_description_set_variant(desc, pangoVariant);
    }
    auto vWeight = style.FontWeight();
    if (vWeight.has_value())
    {
        // cast enum class Lv2cFontWeight to enum PangoWeight!
        PangoWeight pangoWeight = (PangoWeight)(int)(vWeight.value());
        pango_font_description_set_weight(desc, pangoWeight);
    }
    auto vStyle = style.FontStyle();
    if (vStyle.has_value())
    {
        // cast enum class Lv2cFontStyle to enum PangoStyle.
        PangoStyle pangoStyle = (PangoStyle)(int)(vStyle.value());
        pango_font_description_set_style(desc, pangoStyle);
    }
    auto vStretch = style.FontStretch();
    if (vStretch.has_value())
    {
        // cast enum class Lv2cFontStretch to enum PangoStretch.
        PangoStretch pangoStretch = (PangoStretch)(int)(vStretch.value());
        pango_font_description_set_stretch(desc, pangoStretch);
    }
    return desc;

}
