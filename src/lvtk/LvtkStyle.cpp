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

#include "lvtk/LvtkStyle.hpp"
#include "lvtk/LvtkElement.hpp"
#include "lvtk/LvtkTheme.hpp"

using namespace lvtk;

template <typename T>
T LvtkStyle::FromSelfOrClassesOrParent(InheritOptionalPtr<T> pMember, T defaultValue) const
{
    std::optional<T> result = FromSelfOrClassesOrParent<T>(pMember);
    return result.has_value() ? result.value() : defaultValue;
}

void LvtkStyle::SetStyleContext(LvtkSize elementSize)
{
    this->elementSize = elementSize;
}

void LvtkStyle::SetHorizontalStyleContext(double width)
{
    this->elementSize.Width(width);
}
void LvtkStyle::SetVerticalStyleContext(double height)
{
    this->elementSize.Height(height);
}

template <typename T>
T LvtkStyle::FromSelfOrClassesT(std::optional<T> LvtkStyle::*pMember, T defaultValue) const
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

LvtkStyle &LvtkStyle::Margin(const LvtkThicknessMeasurement &value)
{
    margin = value;
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidth(const LvtkThicknessMeasurement &value)
{
    borderWidth = value;
    return *this;
}
LvtkStyle &LvtkStyle::Padding(const LvtkThicknessMeasurement &value)
{
    padding = value;
    return *this;
}
LvtkStyle &LvtkStyle::CellPadding(const LvtkThicknessMeasurement &value)
{
    cellPadding = value;
    return *this;
}

LvtkStyle &LvtkStyle::BorderColor(const LvtkPattern &pattern)
{
    borderColor = pattern;
    return *this;
}
LvtkStyle &LvtkStyle::Background(const LvtkPattern &pattern)
{
    background = pattern;
    return *this;
}
LvtkStyle &LvtkStyle::Color(const LvtkPattern &pattern)
{
    color = pattern;
    return *this;
}
LvtkStyle &LvtkStyle::TintColor(const LvtkPattern &pattern)
{
    tintColor = pattern;
    return *this;
}

LvtkStyle &LvtkStyle::Left(const LvtkMeasurement &value)
{
    left = value;
    return *this;
}
LvtkStyle &LvtkStyle::Top(const LvtkMeasurement &value)
{
    top = value;
    return *this;
}
LvtkStyle &LvtkStyle::Right(const LvtkMeasurement &value)
{
    right = value;
    return *this;
}
LvtkStyle &LvtkStyle::Bottom(const LvtkMeasurement &value)
{
    bottom = value;
    return *this;
}
LvtkStyle &LvtkStyle::Width(const LvtkMeasurement &value)
{
    width = value;
    return *this;
}
LvtkStyle &LvtkStyle::Height(const LvtkMeasurement &value)
{
    height = value;
    return *this;
}
LvtkStyle &LvtkStyle::FontSize(const LvtkMeasurement &value)
{
    fontSize = value;
    return *this;
}

LvtkMeasurement LvtkStyle::Left() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::left);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
LvtkMeasurement LvtkStyle::Top() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::top);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
LvtkMeasurement LvtkStyle::Bottom() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::bottom);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
LvtkMeasurement LvtkStyle::Right() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::right);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
LvtkMeasurement LvtkStyle::Width() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::width);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}
LvtkMeasurement LvtkStyle::Height() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::height);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}
LvtkMeasurement LvtkStyle::FontSize() const
{
    return FromSelfOrClassesOrParent(&LvtkStyle::fontSize);
}

LvtkThicknessMeasurement LvtkStyle::Margin() const
{
    std::optional<LvtkThicknessMeasurement> value = FromSelfOrClasses<LvtkThicknessMeasurement>(&LvtkStyle::margin);
    if (value.has_value())
    {
        LvtkThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    LvtkThicknessMeasurement result;
    return result;
}
LvtkThicknessMeasurement LvtkStyle::Padding() const
{
    std::optional<LvtkThicknessMeasurement> value = FromSelfOrClasses<LvtkThicknessMeasurement>(&LvtkStyle::padding);
    if (value.has_value())
    {
        LvtkThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    LvtkThicknessMeasurement result;
    return result;
}

LvtkThicknessMeasurement LvtkStyle::CellPadding() const
{
    std::optional<LvtkThicknessMeasurement> value = FromSelfOrClasses<LvtkThicknessMeasurement>(&LvtkStyle::cellPadding);
    if (value.has_value())
    {
        LvtkThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    LvtkThicknessMeasurement result;
    return result;
}

LvtkThicknessMeasurement LvtkStyle::BorderWidth() const
{
    std::optional<LvtkThicknessMeasurement> value = FromSelfOrClasses<LvtkThicknessMeasurement>(&LvtkStyle::borderWidth);
    if (value.has_value())
    {
        LvtkThicknessMeasurement result = value.value();
        result.ResolvePercent(this->elementSize);
        return result;
    }
    LvtkThicknessMeasurement result;
    return result;
}

const LvtkPattern &LvtkStyle::BorderColor() const
{
    InheritPatternPtr pMember = &LvtkStyle::borderColor;
    return FromSelfOrClasses(pMember);
}
const LvtkPattern &LvtkStyle::Background() const
{
    InheritPatternPtr pMember = &LvtkStyle::background;
    return FromSelfOrClasses(pMember);
}

LvtkVisibility LvtkStyle::Visibility() const
{
    return FromSelfOrClassesT<LvtkVisibility>(&LvtkStyle::visibility, LvtkVisibility::Visible);
}

const LvtkPattern &LvtkStyle::Color() const
{
    InheritPatternPtr pMember = &LvtkStyle::color;
    return FromSelfOrClassesOrParent(pMember);
}
const LvtkPattern &LvtkStyle::TintColor() const
{
    InheritPatternPtr pMember = &LvtkStyle::tintColor;
    return FromSelfOrClassesOrParent(pMember);
}

LvtkStyle &LvtkStyle::FontFamily(const std::string &value)
{
    this->fontFamily = value;
    return *this;
}

const std::string &LvtkStyle::FontFamily() const
{

    InheritStringPtr pMember = &LvtkStyle::fontFamily;
    return FromSelfOrClassesOrParent(pMember);
}

void LvtkStyle::SetElement(LvtkElement *element)
{
    this->element = element;
}

LvtkStyle &LvtkStyle::HorizontalAlignment(LvtkAlignment alignment)
{
    horizontalAlignment = alignment;
    return *this;
}
LvtkStyle &LvtkStyle::VerticalAlignment(LvtkAlignment alignment)
{
    verticalAlignment = alignment;
    return *this;
}
LvtkAlignment LvtkStyle::HorizontalAlignment() const
{
    return FromSelfOrClasses(&LvtkStyle::horizontalAlignment, LvtkAlignment::Start);
}
LvtkAlignment LvtkStyle::VerticalAlignment() const
{
    return FromSelfOrClasses(&LvtkStyle::verticalAlignment, LvtkAlignment::Start);
}

LvtkStyle &LvtkStyle::Visibility(LvtkVisibility visibility)
{
    if (this->visibility != visibility)
    {
        bool hadLayout = this->visibility != LvtkVisibility::Collapsed;

        this->visibility = visibility;

        bool hasLayout = this->visibility != LvtkVisibility::Collapsed;
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

const LvtkMeasurement &LvtkStyle::FromSelfOrClasses(InheritMeasurementPtr pMember) const
{
    const LvtkMeasurement &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const LvtkMeasurement &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }
        }
    }
    return result;
}

const LvtkMeasurement &LvtkStyle::FromSelfOrClassesOrParent(InheritMeasurementPtr pMember) const
{
    const LvtkMeasurement &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const LvtkMeasurement &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const LvtkMeasurement &parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (!parentResult.isEmpty())
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

const LvtkPattern &LvtkStyle::FromSelfOrClasses(InheritPatternPtr pMember) const
{
    const LvtkPattern &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const LvtkPattern &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }
        }
    }
    return result;
}

const LvtkPattern &LvtkStyle::FromSelfOrClassesOrParent(InheritPatternPtr pMember) const
{
    const LvtkPattern &result = (this->*pMember);
    if (result.isEmpty())
    {
        if (this->element)
        {

            for (const auto &class_ : element->Classes())
            {
                const LvtkPattern &classResult = (class_.get()->*pMember);
                if (!classResult.isEmpty())
                {
                    return classResult;
                }
            }

            if (element->Parent())
            {
                const LvtkPattern &parentResult = element->Parent()->Style().FromSelfOrClassesOrParent(pMember);
                if (!parentResult.isEmpty())
                {
                    return parentResult;
                }
            }
        }
    }
    return result;
}

const std::string &LvtkStyle::FromSelfOrClassesOrParent(InheritStringPtr pMember) const
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
inline std::shared_ptr<T> LvtkStyle::FromSelfOrClassesOrParent(InheritOptionalSharedPtr<T> pMember) const
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
inline std::optional<T> LvtkStyle::FromSelfOrClassesOrParent(LvtkStyle::InheritOptionalPtr<T> pMember) const
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
inline std::optional<T> LvtkStyle::FromSelfOrClasses(LvtkStyle::InheritOptionalPtr<T> pMember) const
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
inline T LvtkStyle::FromSelfOrClasses(LvtkStyle::InheritOptionalPtr<T> pMember, T defaultValue) const
{
    std::optional<T> result = FromSelfOrClasses(pMember);
    if (!result.has_value())
    {
        return defaultValue;
    }
    return result.value();
}

LvtkStyle &LvtkStyle::TextAlign(LvtkTextAlign value)
{
    this->textAlign = value;
    return *this;
}
std::optional<LvtkTextAlign> LvtkStyle::TextAlignOptional() const
{
    auto ptr = &LvtkStyle::textAlign;
    return FromSelfOrClassesOrParent(ptr);
}
LvtkTextAlign LvtkStyle::TextAlign() const
{
    auto v = TextAlignOptional();
    if (v.has_value())
    {
        return v.value();
    }
    return LvtkTextAlign::Start;
}

std::optional<LvtkFontWeight> LvtkStyle::FontWeight()
{
    auto ptr = &LvtkStyle::fontWeight;
    return FromSelfOrClassesOrParent(ptr);
}
std::optional<LvtkFontStretch> LvtkStyle::FontStretch()
{
    auto ptr = &LvtkStyle::fontStretch;
    return FromSelfOrClassesOrParent(ptr);
}

std::optional<LvtkFontStyle> LvtkStyle::FontStyle()
{
    auto ptr = &LvtkStyle::fontStyle;
    return FromSelfOrClassesOrParent(ptr);
}
std::optional<LvtkFontVariant> LvtkStyle::FontVariant()
{
    auto ptr = &LvtkStyle::fontVariant;
    return FromSelfOrClassesOrParent(ptr);
}

LvtkStyle &LvtkStyle::FlexDirection(LvtkFlexDirection flexDirection)
{
    this->flexDirection = flexDirection;
    return *this;
}
LvtkFlexDirection LvtkStyle::FlexDirection() const
{
    return FromSelfOrClasses(&LvtkStyle::flexDirection, LvtkFlexDirection::Row);
}

LvtkStyle &LvtkStyle::FlexWrap(LvtkFlexWrap flexWrap)
{
    this->flexWrap = flexWrap;
    return *this;
}

LvtkFlexWrap LvtkStyle::FlexWrap() const
{
    return FromSelfOrClasses(&LvtkStyle::flexWrap, LvtkFlexWrap::NoWrap);
}

LvtkStyle &LvtkStyle::FlexJustification(LvtkFlexJustification flexJustification)
{
    this->flexJustification = flexJustification;
    return *this;
}

LvtkFlexJustification LvtkStyle::FlexJustification() const
{
    return FromSelfOrClasses(&LvtkStyle::flexJustification, LvtkFlexJustification::Start);
}


LvtkStyle &LvtkStyle::FlexOverflowJustification(LvtkFlexOverflowJustification flexOverflowJustification)
{
    this->flexOverflowJustification = flexOverflowJustification;
    return *this;
}

LvtkFlexOverflowJustification LvtkStyle::FlexOverflowJustification() const
{
    return FromSelfOrClasses(&LvtkStyle::flexOverflowJustification, LvtkFlexOverflowJustification::Normal);
}

LvtkStyle &LvtkStyle::FlexAlignItems(LvtkAlignment flexAlignItems)
{
    this->flexAlignItems = flexAlignItems;
    return *this;
}
LvtkAlignment LvtkStyle::FlexAlignItems() const
{
    return FromSelfOrClasses(&LvtkStyle::flexAlignItems, LvtkAlignment::Start);
}

LvtkStyle &LvtkStyle::Theme(std::shared_ptr<LvtkTheme> theme)
{
    this->theme = theme;
    return *this;
}
static LvtkTheme::ptr defaultTheme = LvtkTheme::Create();

const LvtkTheme &LvtkStyle::Theme() const
{
    LvtkTheme::ptr result = FromSelfOrClassesOrParent(&LvtkStyle::theme);
    if (!result)
    {
        return *(defaultTheme.get());
    }
    return *(result.get());
}

LvtkStyle &LvtkStyle::RoundCorners(const LvtkRoundCornersMeasurement &value)
{
    this->roundCorners = value;
    return *this;
}
LvtkRoundCornersMeasurement LvtkStyle::RoundCorners() const
{
    auto result = FromSelfOrClasses(&LvtkStyle::roundCorners);
    if (result.has_value())
    {
        LvtkRoundCornersMeasurement t = result.value();
        t.ResolvePercent(this->elementSize);
        return t;
    }
    return LvtkRoundCornersMeasurement();
}

LvtkStyle &LvtkStyle::Opacity(double value)
{
    this->opacity = value;
    return *this;
}
double LvtkStyle::Opacity() const
{
    return FromSelfOrClasses(&LvtkStyle::opacity, 1.0);
}

LvtkStyle &LvtkStyle::MarginLeft(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::MarginTop(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Top(value);
    return *this;
}
LvtkStyle &LvtkStyle::MarginRight(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::MarginStart(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    // stubbed in for now.
    margin.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::MarginEnd(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    // stubbed in for now.
    margin.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::MarginBottom(const LvtkMeasurement &value)
{
    if (!margin.has_value())
    {
        margin = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    margin.value().Bottom(value);
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidthLeft(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidthTop(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Top(value);
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidthRight(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Right(value);
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidthStart(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::BorderWidthEnd(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::BorderWidthBottom(const LvtkMeasurement &value)
{
    if (!borderWidth.has_value())
    {
        borderWidth = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    borderWidth.value().Bottom(value);
    return *this;
}
LvtkStyle &LvtkStyle::PaddingLeft(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::PaddingTop(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Top(value);
    return *this;
}
LvtkStyle &LvtkStyle::PaddingRight(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::PaddingStart(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::PaddingEnd(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::PaddingBottom(const LvtkMeasurement &value)
{
    if (!padding.has_value())
    {
        padding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    padding.value().Bottom(value);
    return *this;
}

LvtkStyle &LvtkStyle::RowGap(const LvtkMeasurement &value)
{
    flexRowGap = value;
    return *this;
}
LvtkMeasurement LvtkStyle::RowGap() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::flexRowGap);
    result.ResolvePercent(this->elementSize.Height());
    return result;
}

LvtkStyle &LvtkStyle::ColumnGap(const LvtkMeasurement &value)
{
    flexColumnGap = value;
    return *this;
}
LvtkMeasurement LvtkStyle::ColumnGap() const
{
    LvtkMeasurement result = FromSelfOrClasses(&LvtkStyle::flexColumnGap);
    result.ResolvePercent(this->elementSize.Width());
    return result;
}

LvtkStyle &LvtkStyle::SingleLine(bool value)
{
    this->singleLine = value;
    return *this;
}
bool LvtkStyle::SingleLine() const
{
    return FromSelfOrClassesOrParent<bool>(&LvtkStyle::singleLine, true);
}

LvtkStyle &LvtkStyle::Ellipsize(LvtkEllipsizeMode ellipsize)
{
    this->ellipsizeMode = ellipsize;
    return *this;
}
LvtkEllipsizeMode LvtkStyle::Ellipsize() const
{
    return FromSelfOrClassesOrParent<LvtkEllipsizeMode>(&LvtkStyle::ellipsizeMode, LvtkEllipsizeMode::Disable);
}

double LvtkStyle::LineSpacing() const {
    return FromSelfOrClassesOrParent<double>(&LvtkStyle::lineSpacing, 1.0);
}

LvtkStyle&LvtkStyle::LineSpacing(double value)
{
    this->lineSpacing = value; return *this;
}


LvtkStyle&LvtkStyle::TextTransform(LvtkTextTransform value)
{
    this->textTransform = value; return *this;
}
LvtkTextTransform LvtkStyle::TextTransform() const
{
    return FromSelfOrClassesOrParent<LvtkTextTransform>(&LvtkStyle::textTransform,LvtkTextTransform::Normal);
}

LvtkStyle&LvtkStyle::IconSize(const std::optional<double>& value)
{
    this->iconSize = value;
    return *this;
}
double LvtkStyle::IconSize() const
{
    return FromSelfOrClassesOrParent<double>(&LvtkStyle::iconSize,0);
}
LvtkStyle&LvtkStyle::MinWidth(const std::optional<LvtkMeasurement>& value)
{
    this->minWidth = value;
    return *this;
}
std::optional<LvtkMeasurement> LvtkStyle::MinWidth() const
{
    auto result = FromSelfOrClasses<LvtkMeasurement>(&LvtkStyle::minWidth);
    if (result.has_value())
    {
        result.value().ResolvePercent(this->elementSize.Width());
    }
    return result;

}

LvtkStyle&LvtkStyle::MaxWidth(const std::optional<LvtkMeasurement>& value)
{
    this->maxWidth = value;
    return *this;
}
std::optional<LvtkMeasurement> LvtkStyle::MaxWidth() const
{
    auto result =  FromSelfOrClasses<LvtkMeasurement>(&LvtkStyle::maxWidth);
    if (result.has_value())
    {
        result.value().ResolvePercent(this->elementSize.Width());
    }
    return result;
}

LvtkStyle &LvtkStyle::CellPaddingLeft(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::CellPaddingTop(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Top(value);
    return *this;
}
LvtkStyle &LvtkStyle::CellPaddingRight(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::CellPaddingStart(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Left(value);
    return *this;
}
LvtkStyle &LvtkStyle::CellPaddingEnd(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Right(value);
    return *this;
}

LvtkStyle &LvtkStyle::CellPaddingBottom(const LvtkMeasurement &value)
{
    if (!cellPadding.has_value())
    {
        cellPadding = LvtkThicknessMeasurement(0, 0, 0, 0);
    }
    cellPadding.value().Bottom(value);
    return *this;
}


