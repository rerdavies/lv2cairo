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

#include "lv2c/Lv2cStyle.hpp"
#include "lv2c/Lv2cElement.hpp"
#include "lv2c/Lv2cTheme.hpp"

using namespace lvtk;

template <typename T>
T Lv2cStyle::FromSelfOrClassesOrParent(InheritOptionalPtr<T> pMember, T defaultValue) const
{
    std::optional<T> result = FromSelfOrClassesOrParent<T>(pMember);
    return result.has_value() ? result.value() : defaultValue;
}

void Lv2cStyle::SetStyleContext(Lv2cSize elementSize)
{
    this->elementSize = elementSize;
}

void Lv2cStyle::SetHorizontalStyleContext(double width)
{
    this->elementSize.Width(width);
}
void Lv2cStyle::SetVerticalStyleContext(double height)
{
    this->elementSize.Height(height);
}

template <typename T>
T Lv2cStyle::FromSelfOrClassesT(std::optional<T> Lv2cStyle::*pMember, T defaultValue) const
{
    const std::optional<T> &result = (this->*pMember);
    if (result.has_value())
    {
        return result.value();
    }
    if (this->element)
    {

        for (const auto &class_ : element->Classes())
        {
            const std::optional<T> &classResult = (class_.get()->*pMember);
            if (classResult.has_value())
            {
                return classResult.value();
            }
        }
    }
    return defaultValue;
}

Lv2cStyle &Lv2cStyle::Margin(const Lv2cThicknessMeasurement &value)
{
    margin = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidth(const Lv2cThicknessMeasurement &value)
{
    borderWidth = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Padding(const Lv2cThicknessMeasurement &value)
{
    padding = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::CellPadding(const Lv2cThicknessMeasurement &value)
{
    cellPadding = value;
    return *this;
}

Lv2cStyle &Lv2cStyle::BorderColor(const Lv2cPattern &pattern)
{
    borderColor = pattern;
    return *this;
}
Lv2cStyle &Lv2cStyle::Background(const Lv2cPattern &pattern)
{
    background = pattern;
    return *this;
}
Lv2cStyle &Lv2cStyle::Color(const Lv2cPattern &pattern)
{
    color = pattern;
    return *this;
}
Lv2cStyle &Lv2cStyle::TintColor(const Lv2cPattern &pattern)
{
    tintColor = pattern;
    return *this;
}

Lv2cStyle &Lv2cStyle::Left(const Lv2cMeasurement &value)
{
    left = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Top(const Lv2cMeasurement &value)
{
    top = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Right(const Lv2cMeasurement &value)
{
    right = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Bottom(const Lv2cMeasurement &value)
{
    bottom = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Width(const Lv2cMeasurement &value)
{
    width = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::Height(const Lv2cMeasurement &value)
{
    height = value;
    return *this;
}
Lv2cStyle &Lv2cStyle::FontSize(const Lv2cMeasurement &value)
{
    fontSize = value;
    return *this;
}

Lv2cMeasurement Lv2cStyle::Left() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::left);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
Lv2cMeasurement Lv2cStyle::Top() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::top);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
Lv2cMeasurement Lv2cStyle::Bottom() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::bottom);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
Lv2cMeasurement Lv2cStyle::Right() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::right);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
Lv2cMeasurement Lv2cStyle::Width() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::width);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
Lv2cMeasurement Lv2cStyle::Height() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::height);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
Lv2cMeasurement Lv2cStyle::FontSize() const
{
    return FromSelfOrClassesOrParent(&Lv2cStyle::fontSize);
}

Lv2cThicknessMeasurement Lv2cStyle::Margin() const
{
    std::optional<Lv2cThicknessMeasurement> value = FromSelfOrClasses<Lv2cThicknessMeasurement>(&Lv2cStyle::margin);
    if (value.has_value())
    {
        Lv2cThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    Lv2cThicknessMeasurement result;
    return result;
}
Lv2cThicknessMeasurement Lv2cStyle::Padding() const
{
    std::optional<Lv2cThicknessMeasurement> value = FromSelfOrClasses<Lv2cThicknessMeasurement>(&Lv2cStyle::padding);
    if (value.has_value())
    {
        Lv2cThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    Lv2cThicknessMeasurement result;
    return result;
}

Lv2cThicknessMeasurement Lv2cStyle::CellPadding() const
{
    std::optional<Lv2cThicknessMeasurement> value = FromSelfOrClasses<Lv2cThicknessMeasurement>(&Lv2cStyle::cellPadding);
    if (value.has_value())
    {
        Lv2cThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    Lv2cThicknessMeasurement result;
    return result;
}

Lv2cThicknessMeasurement Lv2cStyle::BorderWidth() const
{
    std::optional<Lv2cThicknessMeasurement> value = FromSelfOrClasses<Lv2cThicknessMeasurement>(&Lv2cStyle::borderWidth);
    if (value.has_value())
    {
        Lv2cThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    Lv2cThicknessMeasurement result;
    return result;
}

const Lv2cPattern &Lv2cStyle::BorderColor() const
{
    InheritPatternPtr pMember = &Lv2cStyle::borderColor;
    return FromSelfOrClasses(pMember);
}
const Lv2cPattern &Lv2cStyle::Background() const
{
    InheritPatternPtr pMember = &Lv2cStyle::background;
    return FromSelfOrClasses(pMember);
}

Lv2cVisibility Lv2cStyle::Visibility() const
{
    return FromSelfOrClassesT<Lv2cVisibility>(&Lv2cStyle::visibility, Lv2cVisibility::Visible);
}

const Lv2cPattern &Lv2cStyle::Color() const
{
    InheritPatternPtr pMember = &Lv2cStyle::color;
    return FromSelfOrClassesOrParent(pMember);
}
const Lv2cPattern &Lv2cStyle::TintColor() const
{
    InheritPatternPtr pMember = &Lv2cStyle::tintColor;
    return FromSelfOrClassesOrParent(pMember);
}

Lv2cStyle &Lv2cStyle::FontFamily(const std::string &value)
{
    this->fontFamily = value;
    return *this;
}

const std::string &Lv2cStyle::FontFamily() const
{

    InheritStringPtr pMember = &Lv2cStyle::fontFamily;
    return FromSelfOrClassesOrParent(pMember);
}

void Lv2cStyle::SetElement(Lv2cElement *element)
{
    this->element = element;
}

Lv2cStyle &Lv2cStyle::HorizontalAlignment(Lv2cAlignment alignment)
{
    horizontalAlignment = alignment;
    return *this;
}
Lv2cStyle &Lv2cStyle::VerticalAlignment(Lv2cAlignment alignment)
{
    verticalAlignment = alignment;
    return *this;
}
Lv2cAlignment Lv2cStyle::HorizontalAlignment() const
{
    return FromSelfOrClasses(&Lv2cStyle::horizontalAlignment, Lv2cAlignment::Start);
}
Lv2cAlignment Lv2cStyle::VerticalAlignment() const
{
    return FromSelfOrClasses(&Lv2cStyle::verticalAlignment, Lv2cAlignment::Start);
}

Lv2cStyle &Lv2cStyle::Visibility(Lv2cVisibility visibility)
{
    if (this->visibility != visibility)
    {
        bool hadLayout = this->visibility != Lv2cVisibility::Collapsed;

        this->visibility = visibility;

        bool hasLayout = this->visibility != Lv2cVisibility::Collapsed;
        if (element)
        {
            if (hasLayout != hadLayout)
            {
                element->InvalidateLayout();
            }
            else
            {
                element->Invalidate();
            }
        }
    }
    return *this;
}

const Lv2cMeasurement &Lv2cStyle::FromSelfOrClasses(InheritMeasurementPtr pMember) const
{
    const Lv2cMeasurement &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const Lv2cMeasurement &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }
        }
    }
    return result;
}

const Lv2cMeasurement &Lv2cStyle::FromSelfOrClassesOrParent(InheritMeasurementPtr pMember) const
{
    const Lv2cMeasurement &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const Lv2cMeasurement &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const Lv2cMeasurement &parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (!parentResult.isEmpty())
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

const Lv2cPattern &Lv2cStyle::FromSelfOrClasses(InheritPatternPtr pMember) const
{
    const Lv2cPattern &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const Lv2cPattern &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }
        }
    }
    return result;
}

const Lv2cPattern &Lv2cStyle::FromSelfOrClassesOrParent(InheritPatternPtr pMember) const
{
    const Lv2cPattern &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const Lv2cPattern &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const Lv2cPattern &parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (!parentResult.isEmpty())
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

const std::string &Lv2cStyle::FromSelfOrClassesOrParent(InheritStringPtr pMember) const
{
    const std::string &result = (this->*pMember);
    if (result.length() == 0)
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const std::string &classResult = (class_.get()->*pMember);
                if (classResult.length() != 0)
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const std::string &parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (parentResult.length() != 0)
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

template <typename T>
inline std::shared_ptr<T> Lv2cStyle::FromSelfOrClassesOrParent(InheritOptionalSharedPtr<T> pMember) const
{
    std::shared_ptr<T> result = (this->*pMember);
    if (!result)
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                std::shared_ptr<T> classResult = (class_.get()->*pMember);
                if (classResult)
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const std::shared_ptr<T> parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (parentResult)
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

template <typename T>
inline std::optional<T> Lv2cStyle::FromSelfOrClassesOrParent(Lv2cStyle::InheritOptionalPtr<T> pMember) const
{
    std::optional<T> result = (this->*pMember);
    if (!result.has_value())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                std::optional<T> classResult = (class_.get()->*pMember);
                if (classResult.has_value())
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const std::optional<T> parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (parentResult.has_value())
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

template <typename T>
inline std::optional<T> Lv2cStyle::FromSelfOrClasses(Lv2cStyle::InheritOptionalPtr<T> pMember) const
{
    std::optional<T> result = (this->*pMember);
    if (!result.has_value())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                std::optional<T> classResult = (class_.get()->*pMember);
                if (classResult.has_value())
                {
                    return classResult;
                }
            }
        }
    }
    return result;
}
template <typename T>
inline T Lv2cStyle::FromSelfOrClasses(Lv2cStyle::InheritOptionalPtr<T> pMember, T defaultValue) const
{
    std::optional<T> result = FromSelfOrClasses(pMember);
    if (!result.has_value())
    {
        return defaultValue;
    }
    return result.value();
}

Lv2cStyle &Lv2cStyle::TextAlign(Lv2cTextAlign value)
{
    this->textAlign = value;
    return *this;
}
std::optional<Lv2cTextAlign> Lv2cStyle::TextAlignOptional() const
{
    auto ptr = &Lv2cStyle::textAlign;
    return FromSelfOrClassesOrParent(ptr);
}
Lv2cTextAlign Lv2cStyle::TextAlign() const
{
    auto v = TextAlignOptional();
    if (v.has_value())
    {
        return v.value();
    }
    return Lv2cTextAlign::Start;
}

std::optional<Lv2cFontWeight> Lv2cStyle::FontWeight()
{
    auto ptr = &Lv2cStyle::fontWeight;
    return FromSelfOrClassesOrParent(ptr);
}
std::optional<Lv2cFontStretch> Lv2cStyle::FontStretch()
{
    auto ptr = &Lv2cStyle::fontStretch;
    return FromSelfOrClassesOrParent(ptr);
}

std::optional<Lv2cFontStyle> Lv2cStyle::FontStyle()
{
    auto ptr = &Lv2cStyle::fontStyle;
    return FromSelfOrClassesOrParent(ptr);
}
std::optional<Lv2cFontVariant> Lv2cStyle::FontVariant()
{
    auto ptr = &Lv2cStyle::fontVariant;
    return FromSelfOrClassesOrParent(ptr);
}

Lv2cStyle &Lv2cStyle::FlexDirection(Lv2cFlexDirection flexDirection)
{
    this->flexDirection = flexDirection;
    return *this;
}
Lv2cFlexDirection Lv2cStyle::FlexDirection() const
{
    return FromSelfOrClasses(&Lv2cStyle::flexDirection, Lv2cFlexDirection::Row);
}

Lv2cStyle &Lv2cStyle::FlexWrap(Lv2cFlexWrap flexWrap)
{
    this->flexWrap = flexWrap;
    return *this;
}

Lv2cFlexWrap Lv2cStyle::FlexWrap() const
{
    return FromSelfOrClasses(&Lv2cStyle::flexWrap, Lv2cFlexWrap::NoWrap);
}

Lv2cStyle &Lv2cStyle::FlexJustification(Lv2cFlexJustification flexJustification)
{
    this->flexJustification = flexJustification;
    return *this;
}

Lv2cFlexJustification Lv2cStyle::FlexJustification() const
{
    return FromSelfOrClasses(&Lv2cStyle::flexJustification, Lv2cFlexJustification::Start);
}


Lv2cStyle &Lv2cStyle::FlexOverflowJustification(Lv2cFlexOverflowJustification flexOverflowJustification)
{
    this->flexOverflowJustification = flexOverflowJustification;
    return *this;
}

Lv2cFlexOverflowJustification Lv2cStyle::FlexOverflowJustification() const
{
    return FromSelfOrClasses(&Lv2cStyle::flexOverflowJustification, Lv2cFlexOverflowJustification::Normal);
}

Lv2cStyle &Lv2cStyle::FlexAlignItems(Lv2cAlignment flexAlignItems)
{
    this->flexAlignItems = flexAlignItems;
    return *this;
}
Lv2cAlignment Lv2cStyle::FlexAlignItems() const
{
    return FromSelfOrClasses(&Lv2cStyle::flexAlignItems, Lv2cAlignment::Start);
}

Lv2cStyle &Lv2cStyle::Theme(std::shared_ptr<Lv2cTheme> theme)
{
    this->theme = theme;
    return *this;
}
static Lv2cTheme::ptr defaultTheme = Lv2cTheme::Create();

const Lv2cTheme &Lv2cStyle::Theme() const
{
    Lv2cTheme::ptr result = FromSelfOrClassesOrParent(&Lv2cStyle::theme);
    if (!result)
    {
        return *(defaultTheme.get());
    }
    return *(result.get());
}

Lv2cStyle &Lv2cStyle::RoundCorners(const Lv2cRoundCornersMeasurement &value)
{
    this->roundCorners = value;
    return *this;
}
Lv2cRoundCornersMeasurement Lv2cStyle::RoundCorners() const
{
    auto result = FromSelfOrClasses(&Lv2cStyle::roundCorners);
    if (result.has_value())
    {
        Lv2cRoundCornersMeasurement t = result.value();
        t.ResolvePercent(this->elementSize);
        return t;
    }
    return Lv2cRoundCornersMeasurement();
}

Lv2cStyle &Lv2cStyle::Opacity(double value)
{
    this->opacity = value;
    return *this;
}
double Lv2cStyle::Opacity() const
{
    return FromSelfOrClasses(&Lv2cStyle::opacity, 1.0);
}

Lv2cStyle &Lv2cStyle::MarginLeft(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::MarginTop(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Top(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::MarginRight(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::MarginStart(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    // stubbed in for now.
    margin.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::MarginEnd(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    // stubbed in for now.
    margin.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::MarginBottom(const Lv2cMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Bottom(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidthLeft(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidthTop(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Top(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidthRight(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Right(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidthStart(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::BorderWidthEnd(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::BorderWidthBottom(const Lv2cMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Bottom(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::PaddingLeft(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::PaddingTop(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Top(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::PaddingRight(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::PaddingStart(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::PaddingEnd(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::PaddingBottom(const Lv2cMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Bottom(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::RowGap(const Lv2cMeasurement &value)
{
    flexRowGap = value;
    return *this;
}
Lv2cMeasurement Lv2cStyle::RowGap() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::flexRowGap);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}

Lv2cStyle &Lv2cStyle::ColumnGap(const Lv2cMeasurement &value)
{
    flexColumnGap = value;
    return *this;
}
Lv2cMeasurement Lv2cStyle::ColumnGap() const
{
    Lv2cMeasurement result = FromSelfOrClasses(&Lv2cStyle::flexColumnGap);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}

Lv2cStyle &Lv2cStyle::SingleLine(bool value)
{
    this->singleLine = value;
    return *this;
}
bool Lv2cStyle::SingleLine() const
{
    return FromSelfOrClassesOrParent<bool>(&Lv2cStyle::singleLine, true);
}

Lv2cStyle &Lv2cStyle::Ellipsize(Lv2cEllipsizeMode ellipsize)
{
    this->ellipsizeMode = ellipsize;
    return *this;
}
Lv2cEllipsizeMode Lv2cStyle::Ellipsize() const
{
    return FromSelfOrClassesOrParent<Lv2cEllipsizeMode>(&Lv2cStyle::ellipsizeMode, Lv2cEllipsizeMode::Disable);
}

double Lv2cStyle::LineSpacing() const {
    return FromSelfOrClassesOrParent<double>(&Lv2cStyle::lineSpacing, 1.0);
}

Lv2cStyle&Lv2cStyle::LineSpacing(double value)
{
    this->lineSpacing = value; return *this;
}


Lv2cStyle&Lv2cStyle::TextTransform(Lv2cTextTransform value)
{
    this->textTransform = value; return *this;
}
Lv2cTextTransform Lv2cStyle::TextTransform() const
{
    return FromSelfOrClassesOrParent<Lv2cTextTransform>(&Lv2cStyle::textTransform,Lv2cTextTransform::Normal);
}

Lv2cStyle&Lv2cStyle::IconSize(const std::optional<double>& value)
{
    this->iconSize = value;
    return *this;
}
double Lv2cStyle::IconSize() const
{
    return FromSelfOrClassesOrParent<double>(&Lv2cStyle::iconSize,0);
}
Lv2cStyle&Lv2cStyle::MinWidth(const std::optional<Lv2cMeasurement>& value)
{
    this->minWidth = value;
    return *this;
}
std::optional<Lv2cMeasurement> Lv2cStyle::MinWidth() const
{
    auto result = FromSelfOrClasses<Lv2cMeasurement>(&Lv2cStyle::minWidth);
    if (result.has_value())
    {
        result.value().ResolvePercent(this->elementSize.Width());
    }
    return result;

}

Lv2cStyle&Lv2cStyle::MaxWidth(const std::optional<Lv2cMeasurement>& value)
{
    this->maxWidth = value;
    return *this;
}
std::optional<Lv2cMeasurement> Lv2cStyle::MaxWidth() const
{
    auto result =  FromSelfOrClasses<Lv2cMeasurement>(&Lv2cStyle::maxWidth);
    if (result.has_value())
    {
        result.value().ResolvePercent(this->elementSize.Width());
    }
    return result;
}

Lv2cStyle &Lv2cStyle::CellPaddingLeft(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::CellPaddingTop(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Top(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::CellPaddingRight(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::CellPaddingStart(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Left(value);
    return *this;
}
Lv2cStyle &Lv2cStyle::CellPaddingEnd(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Right(value);
    return *this;
}

Lv2cStyle &Lv2cStyle::CellPaddingBottom(const Lv2cMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = Lv2cThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Bottom(value);
    return *this;
}


