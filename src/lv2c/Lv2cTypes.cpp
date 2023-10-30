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

#include "lv2c/Lv2cTypes.hpp"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include "ss.hpp"
#include <cmath>
#include "lv2c/Lv2cCieColors.hpp"

#include "pango/pango.h"

using namespace lv2c;
using namespace std;

// Make sure Pango and lv2 enum declarations match.

// make sure that enum class lv2c::FontStretch values match typdef enum { } PANGO_STRETCH values.
#define FONT_STRETCH_VALUE_CHECK(PANGO_NAME, LV2CAIRO_NAME) \
    static_assert(int(Lv2cFontStretch::LV2CAIRO_NAME) == PangoStretch::PANGO_STRETCH_##PANGO_NAME);

FONT_STRETCH_VALUE_CHECK(ULTRA_CONDENSED, UltraCondensed)
FONT_STRETCH_VALUE_CHECK(EXTRA_CONDENSED, ExtraCondensed)
FONT_STRETCH_VALUE_CHECK(CONDENSED, Condensed)
FONT_STRETCH_VALUE_CHECK(SEMI_CONDENSED, SemiCondensed)
FONT_STRETCH_VALUE_CHECK(NORMAL, Normal)
FONT_STRETCH_VALUE_CHECK(SEMI_EXPANDED, SemiExpanded)
FONT_STRETCH_VALUE_CHECK(EXPANDED, Expanded)
FONT_STRETCH_VALUE_CHECK(EXTRA_EXPANDED, ExtraExpanded)
FONT_STRETCH_VALUE_CHECK(ULTRA_EXPANDED, UltraExpanded)

#define FONT_STYLE_VALUE_CHECK(PANGO_NAME, LV2CAIRO_NAME) \
    static_assert(int(Lv2cFontStyle::LV2CAIRO_NAME) == PangoStyle::PANGO_STYLE_##PANGO_NAME);

FONT_STYLE_VALUE_CHECK(NORMAL, Normal)
FONT_STYLE_VALUE_CHECK(OBLIQUE, Oblique)
FONT_STYLE_VALUE_CHECK(ITALIC, Italic)

Lv2cRectangle Lv2cRectangle::Union(const Lv2cRectangle &other) const
{
    if (Empty())
        return other;
    if (other.Empty())
        return *this;

    double left = std::min(X(), other.X());
    double right = std::max(Right(), other.Right());
    double top = std::min(Top(), other.Top());
    double bottom = std::max(Bottom(), other.Bottom());
    return Lv2cRectangle(left, top, right - left, bottom - top);
}

Lv2cRectangle Lv2cRectangle::Intersect(const Lv2cRectangle &other) const
{
    double left = std::max(X(), other.X());
    double right = std::min(Right(), other.Right());
    double top = std::max(Top(), other.Top());
    double bottom = std::min(Bottom(), other.Bottom());
    if (right <= left || bottom <= top)
        return Lv2cRectangle(0, 0, 0, 0);
    return Lv2cRectangle(left, top, right - left, bottom - top);
}

double Lv2cMeasurement::PixelValue() const
{
    switch (type_)
    {
    case Type::Empty:
        return 0;
    case Type::Pixels:
        return value;
    case Type::Point:
        return value * (96.0 / 72.0);
    case Type::Percent:
        throw std::runtime_error("Should have been converted by Style getter.");
        // if (this->styleContext == nullptr)
        //     return 0;
        // return value * this->styleContext->length / 100.0;
    default:
        throw std::runtime_error("Invalid value.");
    }
}

bool Lv2cThicknessMeasurement::isEmpty() const { return left.isEmpty() && right.isEmpty() && top.isEmpty() && bottom.isEmpty(); }

bool Lv2cRectangle::Contains(double x, double y) const
{
    return x >= this->x && x < this->x + this->width && y >= this->y && y < this->y + this->height;
}

bool Lv2cRectangle::Contains(Lv2cPoint point) const
{
    return point.x >= this->x && point.x < this->x + this->width && point.y >= this->y && point.y < this->y + this->height;
}

static int readHex(stringstream &s)
{
    int c = s.get();
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else
    {
        throw std::invalid_argument(SS("Invalid hex characater: " << (char)c));
    }
}

static double readC1(stringstream &s)
{
    int v = readHex(s);

    return v / 15.0;
}
static double readC2(stringstream &s)
{
    int v0 = readHex(s);
    int v1 = readHex(s);
    return (v0 * 16 + v1) / 255.0;
}

Lv2cColor::Lv2cColor(const std::string &webColor)
{
    std::stringstream s(webColor);
    int c;

    c = s.get();
    if (c != '#')
    {
        throw std::invalid_argument(SS("Web Color must start with #: " << webColor));
    }
    if (webColor.length() == 4)
    { // #rgb
        a = 1.0;
        r = readC1(s);
        g = readC1(s);
        b = readC1(s);
    }
    else if (webColor.length() == 5)
    { // #rgba
        r = readC1(s);
        g = readC1(s);
        b = readC1(s);
        a = readC1(s);
    }
    else if (webColor.length() == 7)
    {
        //#rrggbb
        a = 1.0;
        r = readC2(s);
        g = readC2(s);
        b = readC2(s);
    }
    else if (webColor.length() == 9)
    {
        //rrggbbaa
        r = readC2(s);
        g = readC2(s);
        b = readC2(s);
        a = readC2(s);
    }
    else
    {
        throw std::invalid_argument(SS("Invalide web color: " << webColor));
    }
}

Lv2cColor &Lv2cColor::operator=(const Lv2cColor &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
    this->a = other.a;
    return *this;
}
Lv2cColor &Lv2cColor::operator=(const std::string &webColor)
{
    Lv2cColor c{webColor};
    *this = c;
    return *this;
}

static inline float vBlend(float blend, float v0, float v1)
{
    return ((1 - blend) * v0 + blend * v1);
}

Lv2cColor Lv2cColor::LinearBlend(double blend, const Lv2cColor &c0, const Lv2cColor &c1)
{
    float vb = (float)blend;
    float r = vBlend(vb, c0.R(), c1.R());
    float g = vBlend(vb, c0.G(), c1.G());
    float b = vBlend(vb, c0.B(), c1.B());
    float a = vBlend(vb, c0.A(), c1.A());
    return Lv2cColor(r, g, b, a);
}

Lv2cHoverColors::Lv2cHoverColors()
{
}
Lv2cHoverColors::Lv2cHoverColors(Lv2cColor background, Lv2cColor foreground)
{
    if (background.A() == 0)
    {
        background = Lv2cColor(foreground.R(), foreground.G(), foreground.B(), 0);
    }
    for (size_t i = 0; i < 8; ++i)
    {
        Lv2cHoverState state = (Lv2cHoverState)i;
        double blend = 0;
        if (state && Lv2cHoverState::Focus)
        {
            blend = 0.15; // 0.35; // 0.5;
        }
        if (state && Lv2cHoverState::Hover)
        {
            blend += 0.15; // 0.35; // 0.6
        }
        if (state && Lv2cHoverState::Pressed)
        {
            blend = 1;
        }
        colors[i] = Lv2cColor::Blend(blend, background, foreground);
    }
}

Lv2cHoverOpacity::Lv2cHoverOpacity()
    : Lv2cHoverOpacity(0.75, 0.85, 0.8, 1.0)
{
}
Lv2cHoverOpacity::Lv2cHoverOpacity(double defaultOpacity, double hoverOpacity, double focusOpacity, double pressedOpacity)
{
    for (size_t i = 0; i < 8; ++i)
    {
        Lv2cHoverState state = (Lv2cHoverState)i;
        double opacity = defaultOpacity;
        if (state && Lv2cHoverState::Focus)
        {
            opacity = focusOpacity;
        }
        if (state && Lv2cHoverState::Hover)
        {
            opacity = hoverOpacity;
        }
        if (state && Lv2cHoverState::Pressed)
        {
            opacity = pressedOpacity;
        }
        this->opacity[i] = opacity;
    }
}

double Lv2cHoverOpacity::GetOpacity(Lv2cHoverState hoverState) const
{
    return opacity[(int)hoverState];
}

static float DeGamma(float value)
{
    float rgb = Lv2cColor::IToRgb(value);
    return rgb;
}

Lv2cHoverColors::Lv2cHoverColors(Lv2cColor textForeground)
    : Lv2cHoverColors(Lv2cColor(textForeground, 0), Lv2cColor(textForeground, DeGamma(0.04)))
{
}
Lv2cHoverColors::Lv2cHoverColors(Lv2cColor textForeground, double maxOpacity)
    : Lv2cHoverColors(Lv2cColor(textForeground, 0), Lv2cColor(textForeground, maxOpacity))
{
}

Lv2cColor Lv2cHoverColors::GetColor(Lv2cHoverState hoverState) const
{
    Lv2cHoverState t = hoverState - Lv2cHoverState::Selected;
    if (hoverState && Lv2cHoverState::Selected) // Selected gets same coloring as pressed.
    {
        t = t + Lv2cHoverState::Pressed;
    }
    return colors[(int)t];
}

Lv2cRoundCornersMeasurement::Lv2cRoundCornersMeasurement()
    : topLeft(0),
      topRight(0),
      bottomLeft(0),
      bottomRight(0)
{
}

Lv2cRoundCornersMeasurement::Lv2cRoundCornersMeasurement(const Lv2cMeasurement &value)
    : topLeft(value),
      topRight(value),
      bottomLeft(value),
      bottomRight(value)
{
}

Lv2cRoundCornersMeasurement::Lv2cRoundCornersMeasurement(
    const Lv2cMeasurement &topLeft,
    const Lv2cMeasurement &topRight,
    const Lv2cMeasurement &bottomLeft,
    const Lv2cMeasurement &bottomRight)
    : topLeft(topLeft),
      topRight(topRight),
      bottomLeft(bottomLeft),
      bottomRight(bottomRight)
{
}

bool Lv2cRoundCornersMeasurement::isEmpty() const
{
    return topLeft.PixelValue() == 0 && topRight.PixelValue() == 0 && bottomLeft.PixelValue() == 0 && bottomRight.PixelValue() == 0;
}

void Lv2cRoundCornersMeasurement::ResolvePercent(Lv2cSize elementBounds)
{
    double size = std::min(elementBounds.Width(), elementBounds.Height());
    topLeft.ResolvePercent(size);
    topRight.ResolvePercent(size);
    bottomLeft.ResolvePercent(size);
    bottomRight.ResolvePercent(size);
}

void Lv2cMeasurement::ResolvePercent(double size)
{
    if (isPercent())
    {
        this->value = size * this->value / 100;
        this->type_ = Type::Pixels;
    }
}

Lv2cRoundCorners Lv2cRoundCornersMeasurement::PixelValue() const
{
    Lv2cRoundCorners result;
    result.topLeft = topLeft.PixelValue();
    result.topRight = topRight.PixelValue();
    result.bottomLeft = bottomLeft.PixelValue();
    result.bottomRight = bottomRight.PixelValue();
    return result;
}

const AnimationHandle AnimationHandle::InvalidHandle;

AnimationHandle AnimationHandle::Next()
{
    AnimationHandle result;
    result.nativeHandle = ++nextHandle;
    return result;
}
uint64_t AnimationHandle::nextHandle = 0;

Lv2cFocusEventArgs::Lv2cFocusEventArgs()
    : oldFocus(nullptr), newFocus(nullptr)
{
}
Lv2cFocusEventArgs::Lv2cFocusEventArgs(Lv2cElement *oldFocus, Lv2cElement *newFocus)
    : oldFocus(oldFocus), newFocus(newFocus)
{
}

std::string Lv2cColor::toString() const
{
    std::stringstream s;
    s << '{' << r << "," << g << "," << b << "," << a << '}';
    return s.str();
}

static const char hexDigits[] = "0123456789ABCDEF";
static void HexValue(std::ostream &s, double value)
{
    int iValue = (int)std::round(value * 255);
    s << hexDigits[iValue / 16];
    s << hexDigits[iValue % 16];
}

std::string Lv2cColor::toPangoString() const
{
    std::stringstream s;
    s << "#";
    HexValue(s, r);
    HexValue(s, g);
    HexValue(s, b);
    HexValue(s, a);
    return s.str();
}
std::string Lv2cColor::ToWebString() const
{

    std::stringstream s;
    s << "#";
    HexValue(s, r);
    HexValue(s, g);
    HexValue(s, b);
    if (a != 1)
    {
        HexValue(s, a);
    }
    return s.str();
}

bool Lv2cColor::operator==(const Lv2cColor &other) const
{
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

/// @brief Inflate a rectangle
/// @param value An mount added to each border of the rectange.
/// @return The inflated rectangle.
Lv2cRectangle Lv2cRectangle::Inflate(double value) const
{

    Lv2cRectangle result{Left() - value, Top() - value, Width() + 2 * value, Height() + 2 * value};
    if (result.width < 0)
        result.width = 0;
    if (result.height < 0)
        result.height = 0;
    return result;
}

Lv2cRectangle Lv2cRectangle::Inflate(double left, double top, double right, double bottom) const
{

    Lv2cRectangle result{
        Left() - left, Top() - top, Width() + left + right, Height() + top + bottom};
    if (result.width < 0)
        result.width = 0;
    if (result.height < 0)
        result.height = 0;
    return result;
}

Lv2cRectangle Lv2cRectangle::Ceiling() const
{
    double left = std::floor(x);
    double right = std::ceil(x + width);
    double top = std::floor(y);
    double bottom = std::ceil(y + height);
    return Lv2cRectangle(left, top, right - left, bottom - top);
}
Lv2cRectangle Lv2cRectangle::Floor() const
{
    double left = std::ceil(x);
    double right = std::floor(x + width);
    double top = std::ceil(y);
    double bottom = std::floor(y + height);
    return Lv2cRectangle(left, top, right - left, bottom - top);
}

/*static*/
double Lv2cPoint::Distance(Lv2cPoint p1, Lv2cPoint p2)
{
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

Lv2cThickness Lv2cThicknessMeasurement::PixelValue() const
{
    return Lv2cThickness{
        .left = Left().PixelValue(),
        .top = Top().PixelValue(),
        .right = Right().PixelValue(),
        .bottom = Bottom().PixelValue()};
}

Lv2cRectangle Lv2cRectangle::Inset(const Lv2cThickness &thickness) const
{
    Lv2cRectangle result = Lv2cRectangle(
        Left() + thickness.left, Top() + thickness.top,
        Width() - thickness.left - thickness.right,
        Height() - thickness.top - thickness.bottom);
    if (result.width < 0)
        result.width = 0;
    if (result.height < 0)
        result.height = 0;
    return result;
}

Lv2cRoundCorners Lv2cRoundCorners::inset(const Lv2cThickness &thickness)
{
    Lv2cRoundCorners result{
        .topLeft = std::max(topLeft - std::max(thickness.left, thickness.top), 0.0),
        .topRight = std::max(topRight - std::max(thickness.right, thickness.top), 0.0),
        .bottomLeft = std::max(bottomLeft - std::max(thickness.bottom, thickness.left), 0.0),
        .bottomRight = std::max(bottomRight - std::max(thickness.right, thickness.bottom), 0.0)};
    return result;
}

Lv2cWindowPosition::Lv2cWindowPosition(
    int x,
    int y,
    int width,
    int height,
    Lv2cWindowState windowState)
    : x(x), y(y), width(width), height(height), windowState(windowState)
{
}

Lv2cWindowPosition::Lv2cWindowPosition()
    : x(0), y(0), width(0), height(0), windowState(Lv2cWindowState::Normal)
{
}

// Convert sRGB value to linear intensity/
/*static */
double Lv2cColor::RgbToI(double value)
{
    if (value < 0)
        return 0;
    if (value > 1)
        return 1;
    if (value < 0.04045)
    {
        return value / 12.92;
    }
    else
    {
        return std::pow((value + 0.055) / 1.055, 2.4);
    }
}

// Convert linear intensity to sRGB
/* static */
double Lv2cColor::IToRgb(double value)
{
    if (value < 0)
        return 0;
    if (value > 1)
        return 1;
    if (value < 0.0031308)
    {
        return value * 12.92;
    }
    else
    {
        return 1.055 * std::pow(value, (1 / 2.4)) - 0.055;
    }
}

static inline double Lerp(double v, double v1, double v2)
{
    return (1-v)*v1+v*v2;
}

/*static*/
Lv2cColor Lv2cColor::Blend(double blend, const Lv2cColor &c0, const Lv2cColor &c1)
{
    Lv2cLinearColor lc0 { c0};
    Lv2cLinearColor lc1 { c1};

    float r = Lerp(blend,lc0.r,lc1.r);
    float g = Lerp(blend, lc0.g,lc1.g);
    float b = Lerp(blend,lc0.b,lc1.b);
    float a = Lerp(blend,lc0.a,lc1.a);
    return Lv2cColor(Lv2cLinearColor(r,g,b,a));
    // double a0 = c0.A();
    // double a1 = c1.A();

    // double aOut = a0 * (1 - blend) + a1 * blend;

    // if (aOut == 0)
    // {
    //     double ri = RgbToI(c0.R()) * (1 - blend) + RgbToI(c1.R()) * blend;
    //     double gi = RgbToI(c0.G()) * (1 - blend) + RgbToI(c1.G()) * blend;
    //     double bi = RgbToI(c0.B()) * (1 - blend) + RgbToI(c1.B()) * blend;

    //     // Not always right, but right sometimes.
    //     return Lv2cColor(
    //         IToRgb(ri),
    //         IToRgb(gi),
    //         IToRgb(bi),
    //         0);
    // }
    // else
    // {
    //     double ri = RgbToI(c0.R()) * a0 * (1 - blend) + RgbToI(c1.R()) * a1 * blend;
    //     double gi = RgbToI(c0.G()) * a0 * (1 - blend) + RgbToI(c1.G()) * a1 * blend;
    //     double bi = RgbToI(c0.B()) * a0 * (1 - blend) + RgbToI(c1.B()) * a1 * blend;
    //     // de-pre-multiply.
    //     double aScale = 1 / aOut;

    //     return Lv2cColor(
    //         IToRgb(ri * aScale),
    //         IToRgb(gi * aScale),
    //         IToRgb(bi * aScale),
    //         aOut);
    // }
}

bool Lv2cHoverColors::operator==(const Lv2cHoverColors &other) const
{
    for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
    {
        if (colors[i] != other.colors[i])
        {
            return false;
        }
    }
    return true;
}

namespace lv2c::implementation
{
    float srgb2i[256];

    float i2srgb[256];

    struct SrgbToITable
    {
        static constexpr size_t inverse_table_size = sizeof(i2srgb) / sizeof(i2srgb[0]);
        static constexpr size_t inverse_table_max = inverse_table_size - 2;

        SrgbToITable()
        {
            for (int i = 0; i < 256; ++i)
            {
                srgb2i[i] = Lv2cColor::RgbToI(i / 255.0);
            }

            for (size_t i = 0; i < inverse_table_size; ++i)
            {
                i2srgb[i] = Lv2cColor::IToRgb(i / (double)inverse_table_max) * 255 + 0.5f; // pre-bias the table to make round-to-zero conversion round to closest value.
            }
        }
    };
    static SrgbToITable rgbConversion;
}

using namespace lv2c::implementation;

/*static*/
void Lv2cLinearColor::ToImageSurface(const std::vector<Lv2cLinearColor> &source, uint8_t *dest)
{
    for (size_t i = 0; i < source.size(); ++i)
    {
        Lv2cLinearColor c = source[i];
        dest[0] = IToSrgb(c.b);
        dest[1] = IToSrgb(c.g);
        dest[2] = IToSrgb(c.r);
        dest[3] = IToSrgb(c.a);

        dest += 4;
    }
}

/*static*/
void Lv2cLinearColor::ToImageSurface(size_t count, const Lv2cLinearColor *source, uint8_t *dest, float scale)
{
    for (size_t i = 0; i < count; ++i)
    {
        Lv2cLinearColor c = source[i] * scale;
        if (c.a <= 0)
        {
            dest[0] = 0;
            dest[1] = 0;
            dest[2] = 0;
            dest[3] = 0;
        }
        else
        {

            dest[0] = IToSrgb(c.b);
            dest[1] = IToSrgb(c.g);
            dest[2] = IToSrgb(c.r);
            dest[3] = IToSrgb(c.a);
        }

        dest += 4;
    }
}

void Lv2cLinearColor::FromImageSurface(size_t count, const uint8_t *source, Lv2cLinearColor *dest)
{
    for (size_t i = 0; i < count; ++i)
    {
        *dest = Lv2cLinearColor::FromImageSurfaceColor(source[2], source[1], source[0], source[3]);
        source += 4;
        ++dest;
    }
}

/*static*/
void Lv2cLinearColor::FromImageSurface(const uint8_t *source, std::vector<Lv2cLinearColor> &dest)
{
    FromImageSurface(dest.size(), source, &(dest[0]));
}

/*static*/
Lv2cLinearColor Lv2cLinearColor::FromSrgb(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
    Lv2cLinearColor result;
    float a = SrgbToI(a_);
    result.a = a;
    result.r = SrgbToI(r_) * a;
    result.g = SrgbToI(g_) * a;
    result.b = SrgbToI(b_) * a;
    return result;
}

/*static*/
Lv2cLinearColor Lv2cLinearColor::FromImageSurfaceColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
    Lv2cLinearColor result;
    float a = SrgbToI(a_);
    result.a = a;
    result.r = SrgbToI(r_);
    result.g = SrgbToI(g_);
    result.b = SrgbToI(b_);
    return result;
}

/*static*/
const Lv2cSize Lv2cSize::Zero{0, 0};

Lv2cLinearColor::Lv2cLinearColor(const Lv2cColor &color)
{

    float a = Lv2cColor::RgbToI(color.A());
    this->a = a;
    this->r = Lv2cColor::RgbToI(color.R()) * a;
    this->g = Lv2cColor::RgbToI(color.G()) * a;
    this->b = Lv2cColor::RgbToI(color.B()) * a;
}

Lv2cLinearColor::Lv2cLinearColor(const Lv2cHsvColor &color)
{
    float r, g, b;
    float hue = color.Hue();
    float saturation = color.Saturation();
    float brightness = color.Brightness();
    while (hue < 0)
        hue += 360;
    hue = fmod(hue, 360);

    float hh = hue / 60.0f;
    int ix = (int)hh;
    float ffrac = hh - ix;

    switch (ix)
    {
    case 0:
    {
        r = brightness;
        g = brightness * (1.0 - (saturation * (1.0 - ffrac)));
        b = brightness * (1.0 - saturation);
        break;
    }
    case 1:
        r = brightness * (1.0 - (saturation * ffrac));
        g = brightness;
        b = brightness * (1.0 - saturation);
        break;
    case 2:
        r = brightness * (1.0 - saturation);
        g = brightness;
        b = brightness * (1.0 - (saturation * (1.0 - ffrac)));
        break;

    case 3:
        r = brightness * (1.0 - saturation);
        g = brightness * (1.0 - (saturation * ffrac));
        b = brightness;
        break;
    case 4:
        r = brightness * (1.0 - (saturation * (1.0 - ffrac)));
        g = brightness * (1.0 - saturation);
        b = brightness;
        break;
    case 5:
    default:
        r = brightness;
        g = brightness * (1.0 - saturation);
        b = brightness * (1.0 - (saturation * ffrac));
        break;
    }
    float a = color.Alpha();
    this->r = r * a;
    this->g = g * a;
    this->b = b * a;
    this->a = a;
}

Lv2cHsvColor::Lv2cHsvColor(float hue, float saturation, float brightness, float alpha)
    : hue(hue), saturation(saturation), brightness(brightness), alpha(alpha)
{
}

Lv2cHsvColor::Lv2cHsvColor(const Lv2cColor &color)
    : Lv2cHsvColor(Lv2cLinearColor(color))
{
}

bool Lv2cHsvColor::operator==(const Lv2cHsvColor &other) const
{
    if (alpha != other.alpha)
        return false;
    if (alpha == 0)
        return true;
    return hue == other.hue && saturation == other.saturation && brightness == other.brightness;
}

Lv2cHsvColor::Lv2cHsvColor(const Lv2cLinearColor &linearColor)
{
    float a = linearColor.a;
    if (a == 0)
    {
        hue = saturation = brightness = alpha = 0;
        return;
    }
    float invA = 1 / a;
    float r = linearColor.r * invA;
    float g = linearColor.g * invA;
    float b = linearColor.b * invA;
    float cmax = std::max(r, std::max(g, b));
    float cmin = std::min(r, std::min(g, b));
    float diff = cmax - cmin;

    if (cmax == cmin)
    {
        hue = 0;
        saturation = 0;
        brightness = cmax;
    }
    else if (cmax == r)
    {
        hue = std::fmod(60 * (g - b) / diff + 360, 360);
    }
    else if (cmax == g)
    {
        hue = std::fmod(60 * ((b - r) / diff) + 120, 360);
    }
    else // if (cmax == b)
    {
        hue = fmod(60 * ((r - g) / diff) + 240, 360);
    }
    if (cmax == 0)
    {
        saturation = 0;
    }
    else
    {
        saturation = diff / cmax;
    }
    brightness = cmax;
    alpha = a;
}

Lv2cColor::Lv2cColor(const Lv2cLinearColor &color)
{
    if (color.a > 0)
    {
        float invA = 1/color.a;
        this->b = Lv2cColor::IToRgb(color.b*invA);
        this->g = Lv2cColor::IToRgb(color.g*invA);
        this->r = Lv2cColor::IToRgb(color.r*invA);
        this->a = Lv2cColor::IToRgb(color.a);
    } else {
        r = g = b = a = 0;
    }
}
Lv2cColor::Lv2cColor(const Lv2cHsvColor &color)
:Lv2cColor(Lv2cLinearColor(color))
{
}


// convenience function to avoid bringing lv2c/Lv2cCieColors.hpp into sope for the entire project.
double Lv2cColor::ColorDifference(const Lv2cColor &c1, const Lv2cColor &c2)
{
    return lv2c::CieColorDifference(c1,c2);
}

static Lv2cCieLab Lerp(double v, const Lv2cCieLab &c1, const Lv2cCieLab&c2)
{
    double L = Lerp(v,c1.L,c2.L);
    double a = Lerp(v,c1.a,c2.a);
    double b = Lerp(v,c1.b,c2.b);
    return Lv2cCieLab(L,a,b);
}
Lv2cColor Lv2cColor::PaletteColor(int level)
{

    Lv2cColor &color = *this;
    if (level <= 500)
    {
        double t = level/500.0;
        Lv2cCieLab whiteLab { Lv2cColor(1,1,1)};
        Lv2cCieLab cLab { color};
        auto labResult = Lerp(t,whiteLab,cLab);
        return labResult.ToLv2cColor();
    } else {
        double t = (level-500)/500.0;
            Lv2cCieLab cLab { color};
        Lv2cCieLab cDark = cLab;
        cDark.L /= 8; // TODO: this can be refactored.
        auto labResult = Lerp(t,cLab,cDark);

        return labResult.ToLv2cColor();
    }
}


bool Lv2cHoverOpacity::operator==(const Lv2cHoverOpacity&other) const
{
    for (size_t i = 0; i < N_ENTRIES; ++i)
    {
        if (opacity[i] != other.opacity[i]) return false;
    }
    return true;
}

Lv2cRoundCorners Lv2cRoundCorners::operator*(double scale)
{
    return Lv2cRoundCorners {
        topLeft*scale,
        topRight*scale,
        bottomLeft*scale,
        bottomRight*scale
    };
}


bool Lv2cRoundCorners::is_empty() const
{
    return topLeft == 0 && topRight == 0 && bottomLeft == 0 && bottomRight == 0; 
}

bool Lv2cDropShadow::operator==(const Lv2cDropShadow&other) const
{
    return variant == other.variant 
    && xOffset == other.xOffset
    && yOffset == other.yOffset
    && radius == other.radius
    && opacity == other.opacity
    && color == other.color;
}
