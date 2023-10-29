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

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "Lv2cTypes.hpp"
#include "Lv2cDrawingContext.hpp"

namespace lvtk
{

    class Lv2cTheme;

    class Lv2cStyle
    {
    public:
        Lv2cStyle()
        {
        }
        using self = Lv2cStyle;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    public:
        // layout
        void SetElement(Lv2cElement *element);
        void SetStyleContext(Lv2cSize elementSize);
        void SetHorizontalStyleContext(double width);
        void SetVerticalStyleContext(double height);

    public:
        // properties.
        self &Theme(std::shared_ptr<Lv2cTheme> theme);
        const Lv2cTheme &Theme() const;

        self &Visibility(Lv2cVisibility visible);
        Lv2cVisibility Visibility() const;

        self &HorizontalAlignment(Lv2cAlignment alignment);
        self &VerticalAlignment(Lv2cAlignment alignment);

        Lv2cThicknessMeasurement Margin() const;
        self &Margin(const Lv2cThicknessMeasurement &value);
        self &MarginLeft(const Lv2cMeasurement &value);
        self &MarginTop(const Lv2cMeasurement &value);
        self &MarginRight(const Lv2cMeasurement &value);
        self &MarginBottom(const Lv2cMeasurement &value);
        self &MarginStart(const Lv2cMeasurement &value);
        self &MarginEnd(const Lv2cMeasurement &value);


        Lv2cThicknessMeasurement BorderWidth() const;
        self &BorderWidth(const Lv2cThicknessMeasurement &value);
        self &BorderWidthLeft(const Lv2cMeasurement &value);
        self &BorderWidthTop(const Lv2cMeasurement &value);
        self &BorderWidthRight(const Lv2cMeasurement &value);
        self &BorderWidthBottom(const Lv2cMeasurement &value);
        self &BorderWidthStart(const Lv2cMeasurement &value);
        self &BorderWidthEnd(const Lv2cMeasurement &value);

        Lv2cThicknessMeasurement Padding() const;
        self &Padding(const Lv2cThicknessMeasurement &value);
        self &PaddingLeft(const Lv2cMeasurement &value);
        self &PaddingTop(const Lv2cMeasurement &value);
        self &PaddingRight(const Lv2cMeasurement &value);
        self &PaddingBottom(const Lv2cMeasurement &value);
        self &PaddingStart(const Lv2cMeasurement &value);
        self &PaddingEnd(const Lv2cMeasurement &value);

        Lv2cThicknessMeasurement CellPadding() const;
        self &CellPadding(const Lv2cThicknessMeasurement &value);
        self &CellPaddingLeft(const Lv2cMeasurement &value);
        self &CellPaddingTop(const Lv2cMeasurement &value);
        self &CellPaddingRight(const Lv2cMeasurement &value);
        self &CellPaddingBottom(const Lv2cMeasurement &value);
        self &CellPaddingStart(const Lv2cMeasurement &value);
        self &CellPaddingEnd(const Lv2cMeasurement &value);


        self &BorderColor(const Lv2cPattern &pattern);
        self &Background(const Lv2cPattern &pattern);
        self &Color(const Lv2cPattern &pattern);
        self &TintColor(const Lv2cPattern &pattern);

        self &Left(const Lv2cMeasurement &value);
        self &Top(const Lv2cMeasurement &value);
        self &Right(const Lv2cMeasurement &value);
        self &Bottom(const Lv2cMeasurement &value);
        self &Width(const Lv2cMeasurement &value);
        self &Height(const Lv2cMeasurement &value);
        self &FontSize(const Lv2cMeasurement &value);
        /// @brief Font Family
        /// @param value
        /// @return
        /// A semi-colon separated list of font names. Works like css font-family.
        self &FontFamily(const std::string &value);
        self &FontWeight(Lv2cFontWeight fontWeight)
        {
            this->fontWeight = fontWeight;
            return *this;
        }
        self &FontStretch(Lv2cFontStretch value)
        {
            this->fontStretch = value;
            return *this;
        }
        self &FontStyle(Lv2cFontStyle value)
        {
            this->fontStyle = value;
            return *this;
        }
        self &FontVariant(Lv2cFontVariant value)
        {
            this->fontVariant = value;
            return *this;
        }

        std::optional<Lv2cFontWeight> FontWeight();
        std::optional<Lv2cFontStretch> FontStretch();
        std::optional<Lv2cFontStyle> FontStyle();
        std::optional<Lv2cFontVariant> FontVariant();

        Lv2cMeasurement Left() const;
        Lv2cMeasurement Top() const;
        Lv2cMeasurement Bottom() const;
        Lv2cMeasurement Right() const;
        Lv2cMeasurement Width() const;
        Lv2cMeasurement Height() const;
        Lv2cMeasurement FontSize() const;



        const Lv2cPattern &Background() const;
        const Lv2cPattern &BorderColor() const;
        const Lv2cPattern &Color() const;
        const Lv2cPattern &TintColor() const;

        const std::string &FontFamily() const;

        Lv2cAlignment HorizontalAlignment() const;
        Lv2cAlignment VerticalAlignment() const;

        self &RoundCorners(const Lv2cRoundCornersMeasurement &value);
        Lv2cRoundCornersMeasurement RoundCorners() const;

        self &FlexDirection(Lv2cFlexDirection flexDirection);
        Lv2cFlexDirection FlexDirection() const;

        self &FlexWrap(Lv2cFlexWrap flexWrap);
        Lv2cFlexWrap FlexWrap() const;

        self &FlexJustification(Lv2cFlexJustification flexJustification);
        Lv2cFlexJustification FlexJustification() const;

        /// @brief How to align content that has overflowed.
        /// @param flexOverflowJustification 
        /// @return Self&
        Lv2cFlexOverflowJustification FlexOverflowJustification() const;
        self &FlexOverflowJustification(Lv2cFlexOverflowJustification flexJustification);


        self &FlexAlignItems(Lv2cAlignment flexAlignItems);
        Lv2cAlignment FlexAlignItems() const;

        self &TextAlign(Lv2cTextAlign value);
        Lv2cTextAlign TextAlign() const;

        self &RowGap(const Lv2cMeasurement &value);
        Lv2cMeasurement RowGap() const;

        self &ColumnGap(const Lv2cMeasurement &value);
        Lv2cMeasurement ColumnGap() const;

        self &Opacity(double value);
        double Opacity() const;

        self &SingleLine(bool value);
        bool SingleLine() const;

        self &Ellipsize(Lv2cEllipsizeMode ellipsize);
        Lv2cEllipsizeMode Ellipsize() const;

        self &LineSpacing(double value);
        double LineSpacing() const;

        self&TextTransform(Lv2cTextTransform value);
        Lv2cTextTransform TextTransform() const;

        self&IconSize(const std::optional<double>& value);
        double IconSize() const;

        self&MinWidth(const std::optional<Lv2cMeasurement>& value);
        std::optional<Lv2cMeasurement> MinWidth() const;

        self&MaxWidth(const std::optional<Lv2cMeasurement>& value);
        std::optional<Lv2cMeasurement> MaxWidth() const;


    private:
        std::optional<Lv2cTextAlign> TextAlignOptional() const;

        template <typename T>
        T FromSelfOrClassesT(std::optional<T> Lv2cStyle::*pMember, T defaultValue) const;

        using InheritMeasurementPtr = Lv2cMeasurement Lv2cStyle::*;
        const Lv2cMeasurement &FromSelfOrClassesOrParent(InheritMeasurementPtr pMember) const;
        const Lv2cMeasurement &FromSelfOrClasses(InheritMeasurementPtr pMember) const;

        using InheritPatternPtr = Lv2cPattern Lv2cStyle::*;
        const Lv2cPattern &FromSelfOrClassesOrParent(InheritPatternPtr pMember) const;

        const Lv2cPattern &FromSelfOrClasses(InheritPatternPtr pMember) const;

        using InheritStringPtr = std::string Lv2cStyle::*;
        const std::string &FromSelfOrClassesOrParent(InheritStringPtr pMember) const;

        template <typename T>
        using InheritOptionalPtr = std::optional<T> Lv2cStyle::*;

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
        using InheritOptionalSharedPtr = std::shared_ptr<T> Lv2cStyle::*;

        template <typename T>
        std::shared_ptr<T> FromSelfOrClassesOrParent(InheritOptionalSharedPtr<T> pMember) const;

        Lv2cElement *element = nullptr;
        const StyleContext *getHorizontalStyleContext() const;
        const StyleContext *getVerticalStyleContext() const;
        void UpdateStyleContexts();

        Lv2cSize elementSize;
        Lv2cMeasurement left, top, right, bottom, width, height, fontSize;
        std::optional<Lv2cThicknessMeasurement> margin;
        std::optional<Lv2cThicknessMeasurement> borderWidth;
        std::optional<Lv2cThicknessMeasurement> padding;
        std::optional<Lv2cThicknessMeasurement> cellPadding;
        Lv2cPattern borderColor;
        Lv2cPattern background;
        Lv2cPattern color;
        Lv2cPattern tintColor;
        std::string fontFamily;
        std::optional<Lv2cVisibility> visibility;

        std::optional<Lv2cFontWeight> fontWeight;
        std::optional<Lv2cFontVariant> fontVariant;
        std::optional<Lv2cFontStyle> fontStyle;
        std::optional<Lv2cFontStretch> fontStretch;

        std::optional<Lv2cAlignment> horizontalAlignment;
        std::optional<Lv2cAlignment> verticalAlignment;

        std::optional<Lv2cFlexDirection> flexDirection;
        std::optional<Lv2cFlexWrap> flexWrap;
        std::optional<Lv2cFlexJustification> flexJustification;
        std::optional<Lv2cFlexOverflowJustification> flexOverflowJustification;
        std::optional<Lv2cAlignment> flexAlignItems;
        Lv2cMeasurement flexRowGap;
        Lv2cMeasurement flexColumnGap;

        std::optional<Lv2cTextAlign> textAlign;
        std::shared_ptr<Lv2cTheme> theme;
        std::optional<Lv2cRoundCornersMeasurement> roundCorners;
        std::optional<double> opacity;
        std::optional<double> iconSize;

        std::optional<Lv2cEllipsizeMode> ellipsizeMode;
        std::optional<bool> singleLine;
        std::optional<double> lineSpacing;
        std::optional<Lv2cTextTransform> textTransform;
        std::optional<Lv2cMeasurement> minWidth;
        std::optional<Lv2cMeasurement> maxWidth;
    };

    /////////

} // namespace