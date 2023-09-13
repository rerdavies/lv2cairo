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

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "LvtkTypes.hpp"
#include "LvtkDrawingContext.hpp"

namespace lvtk
{

    class LvtkTheme;

    class LvtkStyle
    {
    public:
        LvtkStyle()
        {
        }
        using self = LvtkStyle;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    public:
        // layout
        void SetElement(LvtkElement *element);
        void SetStyleContext(LvtkSize elementSize);
        void SetHorizontalStyleContext(double width);
        void SetVerticalStyleContext(double height);

    public:
        // properties.
        self &Theme(std::shared_ptr<LvtkTheme> theme);
        const LvtkTheme &Theme() const;

        self &Visibility(LvtkVisibility visible);
        LvtkVisibility Visibility() const;

        self &HorizontalAlignment(LvtkAlignment alignment);
        self &VerticalAlignment(LvtkAlignment alignment);
        self &Margin(const LvtkThicknessMeasurement &value);
        self &MarginLeft(const LvtkMeasurement &value);
        self &MarginTop(const LvtkMeasurement &value);
        self &MarginRight(const LvtkMeasurement &value);
        self &MarginBottom(const LvtkMeasurement &value);
        self &MarginStart(const LvtkMeasurement &value);
        self &MarginEnd(const LvtkMeasurement &value);

        self &BorderWidth(const LvtkThicknessMeasurement &value);
        self &BorderWidthLeft(const LvtkMeasurement &value);
        self &BorderWidthTop(const LvtkMeasurement &value);
        self &BorderWidthRight(const LvtkMeasurement &value);
        self &BorderWidthBottom(const LvtkMeasurement &value);
        self &BorderWidthStart(const LvtkMeasurement &value);
        self &BorderWidthEnd(const LvtkMeasurement &value);

        self &Padding(const LvtkThicknessMeasurement &value);
        self &PaddingLeft(const LvtkMeasurement &value);
        self &PaddingTop(const LvtkMeasurement &value);
        self &PaddingRight(const LvtkMeasurement &value);
        self &PaddingBottom(const LvtkMeasurement &value);
        self &PaddingStart(const LvtkMeasurement &value);
        self &PaddingEnd(const LvtkMeasurement &value);

        self &BorderColor(const LvtkPattern &pattern);
        self &Background(const LvtkPattern &pattern);
        self &Color(const LvtkPattern &pattern);
        self &TintColor(const LvtkPattern &pattern);

        self &Left(const LvtkMeasurement &value);
        self &Top(const LvtkMeasurement &value);
        self &Right(const LvtkMeasurement &value);
        self &Bottom(const LvtkMeasurement &value);
        self &Width(const LvtkMeasurement &value);
        self &Height(const LvtkMeasurement &value);
        self &FontSize(const LvtkMeasurement &value);
        /// @brief Font Family
        /// @param value
        /// @return
        /// A semi-colon separated list of font names. Works like css font-family.
        self &FontFamily(const std::string &value);
        self &FontWeight(LvtkFontWeight fontWeight)
        {
            this->fontWeight = fontWeight;
            return *this;
        }
        self &FontStretch(LvtkFontStretch value)
        {
            this->fontStretch = value;
            return *this;
        }
        self &FontStyle(LvtkFontStyle value)
        {
            this->fontStyle = value;
            return *this;
        }
        self &FontVariant(LvtkFontVariant value)
        {
            this->fontVariant = value;
            return *this;
        }

        std::optional<LvtkFontWeight> FontWeight();
        std::optional<LvtkFontStretch> FontStretch();
        std::optional<LvtkFontStyle> FontStyle();
        std::optional<LvtkFontVariant> FontVariant();

        LvtkMeasurement Left() const;
        LvtkMeasurement Top() const;
        LvtkMeasurement Bottom() const;
        LvtkMeasurement Right() const;
        LvtkMeasurement Width() const;
        LvtkMeasurement Height() const;
        LvtkMeasurement FontSize() const;

        LvtkThicknessMeasurement Margin() const;

        LvtkThicknessMeasurement Padding() const;
        LvtkThicknessMeasurement BorderWidth() const;

        const LvtkPattern &Background() const;
        const LvtkPattern &BorderColor() const;
        const LvtkPattern &Color() const;
        const LvtkPattern &TintColor() const;

        const std::string &FontFamily() const;

        LvtkAlignment HorizontalAlignment() const;
        LvtkAlignment VerticalAlignment() const;

        self &RoundCorners(const LvtkRoundCornersMeasurement &value);
        LvtkRoundCornersMeasurement RoundCorners() const;

        self &FlexDirection(LvtkFlexDirection flexDirection);
        LvtkFlexDirection FlexDirection() const;

        self &FlexWrap(LvtkFlexWrap flexWrap);
        LvtkFlexWrap FlexWrap() const;

        self &FlexJustification(LvtkFlexJustification flexJustification);
        LvtkFlexJustification FlexJustification() const;

        /// @brief How to align content that has overflowed.
        /// @param flexOverflowJustification 
        /// @return Self&
        LvtkFlexOverflowJustification FlexOverflowJustification() const;
        self &FlexOverflowJustification(LvtkFlexOverflowJustification flexJustification);


        self &FlexAlignItems(LvtkAlignment flexAlignItems);
        LvtkAlignment FlexAlignItems() const;

        self &TextAlign(LvtkTextAlign value);
        LvtkTextAlign TextAlign() const;

        self &FlexRowGap(const LvtkMeasurement &value);
        LvtkMeasurement FlexRowGap() const;

        self &FlexColumnGap(const LvtkMeasurement &value);
        LvtkMeasurement FlexColumnGap() const;

        self &Opacity(double value);
        double Opacity() const;

        self &SingleLine(bool value);
        bool SingleLine() const;

        self &Ellipsize(LvtkEllipsizeMode ellipsize);
        LvtkEllipsizeMode Ellipsize() const;

        self &LineSpacing(double value);
        double LineSpacing() const;

        self&TextTransform(LvtkTextTransform value);
        LvtkTextTransform TextTransform() const;

        self&IconSize(const std::optional<double>& value);
        double IconSize() const;

        self&MinWidth(const std::optional<LvtkMeasurement>& value);
        std::optional<LvtkMeasurement> MinWidth() const;

        self&MaxWidth(const std::optional<LvtkMeasurement>& value);
        std::optional<LvtkMeasurement> MaxWidth() const;


    private:
        std::optional<LvtkTextAlign> TextAlignOptional() const;

        template <typename T>
        T FromSelfOrClassesT(std::optional<T> LvtkStyle::*pMember, T defaultValue) const;

        using InheritMeasurementPtr = LvtkMeasurement LvtkStyle::*;
        const LvtkMeasurement &FromSelfOrClassesOrParent(InheritMeasurementPtr pMember) const;
        const LvtkMeasurement &FromSelfOrClasses(InheritMeasurementPtr pMember) const;

        using InheritPatternPtr = LvtkPattern LvtkStyle::*;
        const LvtkPattern &FromSelfOrClassesOrParent(InheritPatternPtr pMember) const;

        const LvtkPattern &FromSelfOrClasses(InheritPatternPtr pMember) const;

        using InheritStringPtr = std::string LvtkStyle::*;
        const std::string &FromSelfOrClassesOrParent(InheritStringPtr pMember) const;

        template <typename T>
        using InheritOptionalPtr = std::optional<T> LvtkStyle::*;

        // Search style, search classes, search parents.
        template <typename T>
        std::optional<T> FromSelfOrClassesOrParent(InheritOptionalPtr<T> pMember) const;

        template <typename T>
        T FromSelfOrClassesOrParent(InheritOptionalPtr<T> pMember, T defaultValue) const;

        // Search style, search classes, don't search parents.
        template <typename T>
        std::optional<T> FromSelfOrClasses(InheritOptionalPtr<T> pMember) const;

        // Search style, search classes, don't search parents, supply a default value.
        template <typename T>
        T FromSelfOrClasses(InheritOptionalPtr<T> pMember, T defaultValue) const;

        template <typename T>
        using InheritOptionalSharedPtr = std::shared_ptr<T> LvtkStyle::*;

        template <typename T>
        std::shared_ptr<T> FromSelfOrClassesOrParent(InheritOptionalSharedPtr<T> pMember) const;

        LvtkElement *element = nullptr;
        const StyleContext *getHorizontalStyleContext() const;
        const StyleContext *getVerticalStyleContext() const;
        void UpdateStyleContexts();

        LvtkSize elementSize;
        LvtkMeasurement left, top, right, bottom, width, height, fontSize;
        std::optional<LvtkThicknessMeasurement> margin;
        std::optional<LvtkThicknessMeasurement> borderWidth;
        std::optional<LvtkThicknessMeasurement> padding;
        LvtkPattern borderColor;
        LvtkPattern background;
        LvtkPattern color;
        LvtkPattern tintColor;
        std::string fontFamily;
        std::optional<LvtkVisibility> visibility;

        std::optional<LvtkFontWeight> fontWeight;
        std::optional<LvtkFontVariant> fontVariant;
        std::optional<LvtkFontStyle> fontStyle;
        std::optional<LvtkFontStretch> fontStretch;

        std::optional<LvtkAlignment> horizontalAlignment;
        std::optional<LvtkAlignment> verticalAlignment;

        std::optional<LvtkFlexDirection> flexDirection;
        std::optional<LvtkFlexWrap> flexWrap;
        std::optional<LvtkFlexJustification> flexJustification;
        std::optional<LvtkFlexOverflowJustification> flexOverflowJustification;
        std::optional<LvtkAlignment> flexAlignItems;
        LvtkMeasurement flexRowGap;
        LvtkMeasurement flexColumnGap;

        std::optional<LvtkTextAlign> textAlign;
        std::shared_ptr<LvtkTheme> theme;
        std::optional<LvtkRoundCornersMeasurement> roundCorners;
        std::optional<double> opacity;
        std::optional<double> iconSize;

        std::optional<LvtkEllipsizeMode> ellipsizeMode;
        std::optional<bool> singleLine;
        std::optional<double> lineSpacing;
        std::optional<LvtkTextTransform> textTransform;
        std::optional<LvtkMeasurement> minWidth;
        std::optional<LvtkMeasurement> maxWidth;
    };

    /////////

} // namespace