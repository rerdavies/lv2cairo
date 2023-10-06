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

#include "lvtk/LvtkTheme.hpp"

using namespace lvtk;

LvtkTheme::LvtkTheme(const LvtkThemeColors &themeColors)
    : LvtkThemeColors(themeColors)
{
    fontFamily = "Arial,Roboto,Piboto,Liberation sans,Helvetica,Swiss,Sans";

    titleSize = LvtkMeasurement::Point(12);
    headingSize = LvtkMeasurement::Point(12);
    bodySize = LvtkMeasurement::Point(10);
    captionSize = LvtkMeasurement::Point(7);



    menuDropShadow.variant = LvtkDropShadowVariant::DropShadow;
    menuDropShadow.opacity = 0.6;
    menuDropShadow.radius = 6;
    menuDropShadow.xOffset = 1;
    menuDropShadow.yOffset = 4;

    dialDropShadow = LvtkDropShadow {
        .variant = LvtkDropShadowVariant::InnerDropShadow,
        .xOffset = 0.1f,
        .yOffset = 3.0f,
        .radius = 3.0f,
        .opacity = 0.60f,
        .color = LvtkColor(0,0,0)
    };

    toggleThumbDropShadow = LvtkDropShadow{
        .variant = LvtkDropShadowVariant::InnerDropShadow,
        .xOffset = 0.0,
        .yOffset = -2.0,
        .radius = 4.0,
        .opacity = 0.5,
        .color = LvtkColor(0, 0, 0)};
    toggleTrackDropShadow = LvtkDropShadow{
        .variant = LvtkDropShadowVariant::InnerDropShadow,
        .xOffset = 0.1,
        .yOffset = 2.0,
        .radius = 3.0,
        .opacity = 0.95,
    };
    vuSettings.green = vuColor; 
    dbVuSettings.green = "#40C040";
    dbVuSettings.yellow = "#C0C040";
    dbVuSettings.red = "#C04040";

    BuildStyles();
    BuildHoverColors();
}

LvtkTheme::LvtkTheme(bool darkTheme)
    : LvtkTheme(LvtkThemeColors(darkTheme))
{
}

LvtkThemeColors::LvtkThemeColors(bool darkTheme)
:isDarkTheme(darkTheme)
{
    if (darkTheme)
    {
        this->background = "#000000";
        this->paper = "#181818";
        this->dialogBackgroundColor = "#141414";
        this->primaryColor = "#C8C8E0"; //"#8750C4";
        this->secondaryColor = "#E08080";
        this->primaryInvertedTextColor = "#F0F0F0";
        this->secondaryInvertedTextColor = "#F0F0F0";

        this->errorColor = "#FF6666";

        this->popupBackground = "#101010";
        this->dividerColor = "#FFFFFF70";
        this->primaryTextColor = "#F0F0F0";
        this->secondaryTextColor = "#A0A0A0";

        this->textSelectionColor = "#C0C0FF80";
        this->textCursorColor = secondaryTextColor;

        this->dialColor = primaryTextColor;
        toggleTrackColor = this->primaryTextColor;
        toggleThumbColor = "#806c80";

        vuBackground = "#000000";
        vuTickColor = "#A0A0A0"; //"#A0A0A0";
        vuColor = "#6040C0"; //"#202040");

        lampOnColor = vuColor;
        lampOffColor = "#000000";
        portGroupBorderColor = secondaryTextColor;

        plotBackground = vuBackground;
        plotTickColor = "#E0E0E0";
        plotColor = "#00C000C0";
    }
    else
    {
        this->background = "#FFFFFF";
        this->paper = "#E0e0E0";
        this->dialogBackgroundColor = "#F0F0F0";

        this->primaryColor = "#6600CC"; // purple
        this->secondaryColor = "#AA3034"; // Pink
        this->primaryInvertedTextColor = "#F0F0F0";
        this->secondaryInvertedTextColor = "#F0F0F0";

        this->errorColor = "#800000";


        this->popupBackground = "#FFFFFF";
        this->dividerColor = "#808080";
        this->primaryTextColor = "#000000";
        this->secondaryTextColor = "#202020";

        this->textSelectionColor = primaryColor.PaletteColor(300);
        this->textCursorColor = secondaryTextColor;

        this->dialColor = "#A0A0A0"; //primaryColor.PaletteColor(300);
        toggleTrackColor = paper;
        toggleThumbColor = primaryColor.PaletteColor(300);

        vuBackground = "#505050";
        vuTickColor = "#E0E0E0"; //"#A0A0A0";
        vuColor = primaryColor.PaletteColor(300);

        lampOnColor = vuColor;
        lampOffColor = vuBackground;

        portGroupBorderColor = "#606060";

        plotBackground = vuBackground;
        plotTickColor = "#E0E0E0";
        plotColor = "#00C000C0";

    }

}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC optimize("no-var-tracking") // to speed up compilation

void LvtkTheme::BuildStyles()
{

    titleStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontWeight(LvtkFontWeight::Light)
            .FontSize(titleSize)
            .LineSpacing(1.2)
            .Color(secondaryTextColor));
    headingStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontWeight(LvtkFontWeight::Bold)
            .FontSize(headingSize)
            .LineSpacing(1.2)
            .Color(secondaryTextColor));
    bodyPrimaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .LineSpacing(1.2)
            .Color(primaryTextColor));
    bodySecondaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .LineSpacing(1.2)
            .Color(secondaryTextColor));
    captionStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(captionSize)
            .TextTransform(LvtkTextTransform::Capitalize)
            .LineSpacing(1.0)
            .Color(secondaryTextColor));

    buttonDisableStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Opacity(0.7));
    buttonDialogStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(captionSize)
            .Color(secondaryTextColor)
            .TextTransform(LvtkTextTransform::Capitalize)
            .Padding({4})
            .TintColor(secondaryTextColor)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .RoundCorners({4})

    );
    buttonDialogPrimaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(captionSize)
            .TextTransform(LvtkTextTransform::Capitalize)
            .Color(primaryColor)
            .TintColor(LvtkColor(primaryColor, 0.75))

            .Padding({4})
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .RoundCorners({4})

    );
    buttonDialogSecondaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(captionSize)
            .TextTransform(LvtkTextTransform::Capitalize)
            .Color(secondaryColor)
            .TintColor(LvtkColor(secondaryColor, 0.75))
            .Padding({4})
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .RoundCorners({4})

    );

    buttonBorderStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({1})
            .Padding({6, 2, 6, 2})
            .RoundCorners({15})
            .BorderColor(LvtkColor(this->secondaryTextColor, 0.25))
            .TintColor(LvtkColor(secondaryTextColor, 0.75))
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .Color(this->secondaryTextColor));

    buttonBorderDefaultStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({2})
            .Padding({6, 1, 6, 1})
            .RoundCorners({15})
            .BorderColor(LvtkColor(this->secondaryTextColor, 0.25))
            .TintColor(LvtkColor(secondaryTextColor, 0.75))
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .Color(this->secondaryTextColor));

    buttonBorderPrimaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({2})
            .Padding({6, 1, 6, 1})
            .RoundCorners({15})
            .BorderColor(LvtkColor(this->primaryColor, 0.25))
            .TintColor(LvtkColor(primaryColor, 0.75))

            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .Color(this->primaryColor));
    buttonBorderSecondaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({2})
            .Padding({6, 1, 6, 1})
            .RoundCorners({15})
            .BorderColor(LvtkColor(this->secondaryColor, 0.25))
            .TintColor(LvtkColor(secondaryColor, 0.75))
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .Color(this->secondaryColor));

    imageButtonStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .Padding({8})
            .IconSize(24)
            .RoundCorners({20})
            .TintColor(secondaryTextColor)
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .Color(this->secondaryTextColor));

    auto imageButtonPrimaryColor = LvtkColor::LinearBlend(0.75, paper, primaryColor);

    imageButtonPrimaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({0})
            .Padding({4, 4, 4, 4})
            .IconSize(24)
            .RoundCorners({24})
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .TintColor(primaryInvertedTextColor)
            .Color(primaryInvertedTextColor)
            .Background(imageButtonPrimaryColor));

    auto imageButtonSecondaryColor = LvtkColor::LinearBlend(0.75, paper, secondaryColor);
    imageButtonSecondaryStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .FontFamily(fontFamily)
            .FontSize(bodySize)
            .FontVariant(LvtkFontVariant::Normal)
            .BorderWidth({0})
            .Padding({4, 4, 4, 4})
            .IconSize(24)
            .RoundCorners({20})
            .FontSize(this->bodySize)
            .FontStyle(LvtkFontStyle::Normal)
            .SingleLine(true)
            .Ellipsize(LvtkEllipsizeMode::End)
            .TintColor(secondaryInvertedTextColor)
            .Color(secondaryInvertedTextColor)
            .Background(imageButtonSecondaryColor));

    editBoxUnderlineStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .FontFamily(fontFamily)
                .FontSize(bodySize)
                .FontVariant(LvtkFontVariant::Normal)
                .BorderWidth({0, 0, 0, 1})
                .Padding({0, 4, 0, 4})
                .Margin({4, 0, 4, 0})
                .BorderColor(LvtkColor(this->secondaryTextColor, 0.25))
                .SingleLine(true)
                .Color(this->primaryTextColor));
    numericEditBoxStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .TextAlign(LvtkTextAlign::Center));

    editBoxFrameStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .FontFamily(fontFamily)
                .FontSize(bodySize)
                .FontVariant(LvtkFontVariant::Normal)
                .BorderWidth({1})
                .RoundCorners({4})
                .Padding({0, 4, 0, 4})
                .Margin({4, 0, 4, 0})
                .BorderColor(LvtkColor(this->secondaryColor, 0.25))
                .Color(this->primaryTextColor));
    editBoxErrorStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .BorderColor(this->errorColor));

    dropdownUnderlineStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .FontFamily(fontFamily)
                .FontSize(bodySize)
                .FontVariant(LvtkFontVariant::Normal)
                .BorderWidth({0, 0, 0, 1})
                .Padding({4, 4, 0, 4})
                .RoundCorners({4, 6, 0, 0})
                .Margin({4, 1, 4, 0})
                .BorderColor(LvtkColor(this->secondaryTextColor, 0.25))
                .Color(this->primaryTextColor));

    dropdownItemContainerStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .Background(this->popupBackground)
                .BorderColor(this->dividerColor)
                //.BorderWidth({1})
                .Padding({3, 3, 3, 3}));

    dropdownItemStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .FontSize(bodySize)
                .FontVariant(LvtkFontVariant::Normal)
                .Padding({16, 4, 16, 4})
                .Color(this->primaryTextColor)
                .HorizontalAlignment(LvtkAlignment::Stretch));

    dialStyle =
        std::make_shared<LvtkStyle>(
            LvtkStyle()
                .Color(this->dialColor)
                .TintColor(this->dialColor)
                .Width(48)
                .Height(48)
                .Margin(8));
    toggleButtonStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Color(this->primaryColor)
            .Width(54)
            .Height(48)
            .Margin({0, 0, 0, 0})
            .RoundCorners({6}));
    toggleThumbStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Color(this->toggleThumbColor)
            .Height(20)
            .Width(38)
            .RoundCorners({10})
            .VerticalAlignment(LvtkAlignment::Center)
            .HorizontalAlignment(LvtkAlignment::Center)

    );

    toggleTrackStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Color(this->toggleTrackColor)
            .Height(12)
            .Width(24)
            .RoundCorners({6})
            .VerticalAlignment(LvtkAlignment::Center)
            .HorizontalAlignment(LvtkAlignment::Center));
    toggleButtonOffThumbColor = LvtkColor::Blend(0.35, this->paper, this->primaryTextColor);
    toggleButtonOffTrackColor = LvtkColor::Blend(0.25, this->paper, this->primaryTextColor);

    constexpr double VU_BAR_WIDTH = 4;

    vuSettings.padding = 2;

    vuStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Background(vuBackground)
            .Width(VU_BAR_WIDTH + 2 * vuSettings.padding)
            .RoundCorners({2})
            .Height(52)
            .Margin(1));
    stereoVuStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Background(vuBackground)
            .RoundCorners({3})
            .Width(VU_BAR_WIDTH * 2 + 3 * vuSettings.padding)
            .Height(52)
            .Margin(1));

    dbVuSettings.hasTicks = true;
    dbVuSettings.tickDb = 6;
    dbVuSettings.tickWidth = 4;
    dbVuSettings.padding = 2;
    dbVuSettings.redLevel = 0;
    dbVuSettings.yellowLevel = -12;

    dbVuStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Background(vuBackground)
            .RoundCorners({4,3,4,3})
            .Width(VU_BAR_WIDTH + dbVuSettings.tickWidth + 3 * dbVuSettings.padding)
            .Height(48)
            .Margin(1)

    );
    stereoDbVuStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Background(vuBackground)
            .RoundCorners({4,3,4,3})
            .Width(VU_BAR_WIDTH * 2 + dbVuSettings.tickWidth + 4 * dbVuSettings.padding)
            .Height(48)
            .Margin(1)

    );


    lampStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Width(26)
            .Height(26)
    );

    plotStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Background(plotBackground)
            .Width(80)
            .RoundCorners({8})
            .Height(52)
    );

    statusTextStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
        /* nothing to set.*/
    );
    double portGroupBorderWidth = 2;
    double captionTextSizeInPixels = 11;
    double portGroupMarginTop = std::floor((captionTextSizeInPixels - portGroupBorderWidth) / 2);
    double portGroupPaddingTop = 4 + std::ceil(captionTextSizeInPixels) - portGroupMarginTop - portGroupBorderWidth;
    double portGroupPaddingBottom = 4;
    double portGroupMarginBottom = portGroupMarginTop + portGroupPaddingTop - portGroupPaddingBottom;
    portGroupStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .BorderWidth({2})
            .BorderColor(this->portGroupBorderColor)
            .Margin({4, portGroupMarginTop, 4, portGroupMarginBottom})
            .Padding({4, portGroupPaddingTop, 4, portGroupPaddingBottom})
            .RoundCorners({8, 8, 8, 8}));

    scrollbarThumbColors = LvtkHoverColors(
        LvtkColor(this->secondaryTextColor, 0.5),
        LvtkColor(this->secondaryTextColor, 0.75));
    scrollbarTrackColors = LvtkHoverColors(this->secondaryTextColor, 0.12);

    double scrollBarThickness = 8;
    double scrollBarPadding = 4;
    verticalScrollbarStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Width(scrollBarThickness + 2 * scrollBarPadding)
            .Padding({scrollBarPadding})
            .VerticalAlignment(LvtkAlignment::Stretch)
            .HorizontalAlignment(LvtkAlignment::End));
    horizontalScrollbarStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Height(scrollBarThickness + 2 * scrollBarPadding)
            .Padding({scrollBarPadding})
            .VerticalAlignment(LvtkAlignment::End)
            .HorizontalAlignment(LvtkAlignment::Stretch));

    double collapsedBarThickness = 3;
    double collapsedBarPadding = 2;
    // to get the tops to line up.
    double collapsedStartMargin = scrollBarPadding;
    collapsedVerticalScrollbarStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Width(collapsedBarThickness + collapsedBarPadding)
            .Padding({0, collapsedStartMargin, collapsedBarPadding, collapsedStartMargin})
            .Color(LvtkColor(secondaryTextColor, 0.25)));
    collapsedHorizontalScrollbarStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Height(collapsedBarThickness + collapsedBarPadding)
            .Padding({collapsedStartMargin, 0, collapsedStartMargin, collapsedBarPadding})
            .Color(LvtkColor(secondaryTextColor, 0.25)));

    indefiniteProgressStyle = std::make_shared<LvtkStyle>(
        LvtkStyle()
            .Height(2)
            .Color(LvtkColor("#402080")));
}
#pragma GCC diagnostic pop

void LvtkTheme::BuildHoverColors()
{
    this->dialHoverOpacity = LvtkHoverOpacity(0.85,0.92,0.92,1.0);

    this->hoverBackgroundColors = LvtkHoverColors(this->primaryTextColor);
    this->invertedHoverBackgroundColors = LvtkHoverColors(this->paper, 0.2);
    this->primaryHoverBackgroundColors = LvtkHoverColors(this->primaryInvertedTextColor);
    this->secondaryHoverBackgroundColors = LvtkHoverColors(this->secondaryInvertedTextColor);
    this->hoverTextColors = LvtkHoverColors(this->secondaryTextColor, this->primaryTextColor);
    this->dropdownItemHoverBackgroundColors = hoverBackgroundColors;
    // LvtkHoverColors(this->primaryTextColor,0.5);
}
