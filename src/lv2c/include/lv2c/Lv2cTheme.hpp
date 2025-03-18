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


#include <memory>
#include "Lv2cTypes.hpp"
#include "Lv2cStyle.hpp"
#include "Lv2cUserData.hpp"

namespace lv2c {

    class Lv2cThemeColors {
    public:
        Lv2cThemeColors(bool darkTheme);

        bool isDarkTheme;
        Lv2cColor background;
        Lv2cColor paper;

        Lv2cColor popupBackground;
        Lv2cColor dividerColor;


        Lv2cColor primaryColor;
        Lv2cColor primaryInvertedTextColor;
        Lv2cColor secondaryColor;
        Lv2cColor secondaryInvertedTextColor;
        Lv2cColor errorColor;

        Lv2cColor primaryTextColor;
        Lv2cColor secondaryTextColor;


        Lv2cColor dialColor;
        Lv2cColor textSelectionColor;
        Lv2cColor textCursorColor;

        Lv2cColor toggleTrackColor;
        Lv2cColor toggleThumbColor;

        Lv2cColor vuBackground;
        Lv2cColor vuTickColor;
        Lv2cColor vuColor;

        Lv2cColor lampOnColor;
        Lv2cColor lampOffColor;

        Lv2cColor portGroupBorderColor;
        Lv2cColor dialogBackgroundColor;

        Lv2cColor plotBackground;
        Lv2cColor plotTickColor;
        Lv2cColor plotColor;

    };

    class Lv2cTheme: public Lv2cThemeColors {
    public:
        using ptr= std::shared_ptr<Lv2cTheme>;

        static ptr Create(bool darkTheme = true) { return std::make_shared<Lv2cTheme>(darkTheme); }
        static ptr Create(const Lv2cThemeColors &themeColors) { return std::make_shared<Lv2cTheme>(themeColors); }

        Lv2cTheme(bool darkTheme);
        Lv2cTheme(const Lv2cThemeColors &themeColors);

        void BuildStyles();
        void BuildHoverColors();

        std::string fontFamily = "Piboto,Roboto,Arial,Helvetica,Liberation Sans,Swiss,Sans";


        Lv2cMeasurement titleSize;
        Lv2cMeasurement headingSize;
        Lv2cMeasurement bodySize;
        Lv2cMeasurement captionSize;


        Lv2cStyle::ptr titleStyle;
        Lv2cStyle::ptr headingStyle;
        Lv2cStyle::ptr subheadingStyle;
        Lv2cStyle::ptr bodyPrimaryStyle;
        Lv2cStyle::ptr bodySecondaryStyle;
        Lv2cStyle::ptr captionStyle;




        Lv2cHoverColors hoverBackgroundColors;
        Lv2cHoverColors invertedHoverBackgroundColors;
        Lv2cHoverColors primaryHoverBackgroundColors;
        Lv2cHoverColors secondaryHoverBackgroundColors;
        Lv2cHoverColors hoverTextColors;
        Lv2cHoverColors dropdownItemHoverBackgroundColors;

        Lv2cHoverOpacity dialHoverOpacity;


        Lv2cStyle::ptr buttonDialogStyle;
        Lv2cStyle::ptr buttonDialogPrimaryStyle;
        Lv2cStyle::ptr buttonDialogSecondaryStyle;
        Lv2cStyle::ptr buttonBorderStyle;
        Lv2cStyle::ptr buttonBorderDefaultStyle;
        Lv2cStyle::ptr buttonBorderPrimaryStyle;
        Lv2cStyle::ptr buttonBorderSecondaryStyle;

        Lv2cStyle::ptr imageButtonStyle;
        Lv2cStyle::ptr imageButtonPrimaryStyle;
        Lv2cStyle::ptr imageButtonSecondaryStyle;

        Lv2cStyle::ptr filledButtonPrimaryStyle;
        Lv2cStyle::ptr filledButtonSecondaryStyle;

        Lv2cStyle::ptr buttonDisableStyle;



        Lv2cStyle::ptr editBoxUnderlineStyle;
        Lv2cStyle::ptr editBoxFrameStyle;
        Lv2cStyle::ptr editBoxErrorStyle;
        Lv2cStyle::ptr numericEditBoxStyle;

        Lv2cStyle::ptr dropdownUnderlineStyle;
        Lv2cStyle::ptr dropdownItemContainerStyle;
        Lv2cStyle::ptr dropdownItemStyle;

        Lv2cStyle::ptr dialStyle;
        Lv2cStyle::ptr toggleButtonStyle;
        Lv2cStyle::ptr toggleTrackStyle;
        Lv2cStyle::ptr toggleThumbStyle;
        Lv2cColor toggleButtonOffThumbColor;
        Lv2cColor toggleButtonOffTrackColor;
        Lv2cDropShadow toggleThumbDropShadow;
        Lv2cDropShadow toggleTrackDropShadow;

        Lv2cDropShadow menuDropShadow;

        Lv2cDropShadow dialDropShadow;

        Lv2cStyle::ptr vuStyle;
        Lv2cStyle::ptr stereoVuStyle;
        Lv2cStyle::ptr dbVuStyle;
        Lv2cStyle::ptr stereoDbVuStyle;
        Lv2cStyle::ptr progressStyle;
        
        Lv2cVuSettings vuSettings;
        Lv2cVuSettings dbVuSettings;



        Lv2cStyle::ptr lampStyle;

        Lv2cStyle::ptr plotStyle;

        Lv2cStyle::ptr statusTextStyle;

        Lv2cStyle::ptr portGroupStyle;

        Lv2cHoverColors scrollbarThumbColors;
        Lv2cHoverColors scrollbarTrackColors;
        Lv2cStyle::ptr horizontalScrollbarStyle;
        Lv2cStyle::ptr collapsedHorizontalScrollbarStyle;
        Lv2cStyle::ptr verticalScrollbarStyle;
        Lv2cStyle::ptr collapsedVerticalScrollbarStyle;

        Lv2cStyle::ptr indefiniteProgressStyle;

        // A place to store theme-related colors for custom controls. 
        std::map<std::string,Lv2cColor> customColors;
        // A place to store theme-related styles for custom controls. 
        std::map<std::string,Lv2cStyle::ptr> customStyles;
        // A place to store theme-related user daa for custom controls. 
        std::map<std::string,Lv2cUserData::ptr> customUserData;
    };

} // namespace
