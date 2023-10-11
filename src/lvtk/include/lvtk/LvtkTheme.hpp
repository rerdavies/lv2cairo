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
#include "LvtkTypes.hpp"
#include "LvtkStyle.hpp"
#include "LvtkUserData.hpp"

namespace lvtk {

    class LvtkThemeColors {
    public:
        LvtkThemeColors(bool darkTheme);

        bool isDarkTheme;
        LvtkColor background;
        LvtkColor paper;

        LvtkColor popupBackground;
        LvtkColor dividerColor;


        LvtkColor primaryColor;
        LvtkColor primaryInvertedTextColor;
        LvtkColor secondaryColor;
        LvtkColor secondaryInvertedTextColor;
        LvtkColor errorColor;

        LvtkColor primaryTextColor;
        LvtkColor secondaryTextColor;


        LvtkColor dialColor;
        LvtkColor textSelectionColor;
        LvtkColor textCursorColor;

        LvtkColor toggleTrackColor;
        LvtkColor toggleThumbColor;

        LvtkColor vuBackground;
        LvtkColor vuTickColor;
        LvtkColor vuColor;

        LvtkColor lampOnColor;
        LvtkColor lampOffColor;

        LvtkColor portGroupBorderColor;
        LvtkColor dialogBackgroundColor;

        LvtkColor plotBackground;
        LvtkColor plotTickColor;
        LvtkColor plotColor;

    };

    class LvtkTheme: public LvtkThemeColors {
    public:
        using ptr= std::shared_ptr<LvtkTheme>;

        static ptr Create(bool darkTheme = true) { return std::make_shared<LvtkTheme>(darkTheme); }
        static ptr Create(const LvtkThemeColors &themeColors) { return std::make_shared<LvtkTheme>(themeColors); }

        LvtkTheme(bool darkTheme);
        LvtkTheme(const LvtkThemeColors &themeColors);

        void BuildStyles();
        void BuildHoverColors();

        std::string fontFamily = "Piboto,Roboto,Arial,Helvetica,Liberation Sans,Swiss,Sans";


        LvtkMeasurement titleSize;
        LvtkMeasurement headingSize;
        LvtkMeasurement bodySize;
        LvtkMeasurement captionSize;


        LvtkStyle::ptr titleStyle;
        LvtkStyle::ptr headingStyle;
        LvtkStyle::ptr subheadingStyle;
        LvtkStyle::ptr bodyPrimaryStyle;
        LvtkStyle::ptr bodySecondaryStyle;
        LvtkStyle::ptr captionStyle;




        LvtkHoverColors hoverBackgroundColors;
        LvtkHoverColors invertedHoverBackgroundColors;
        LvtkHoverColors primaryHoverBackgroundColors;
        LvtkHoverColors secondaryHoverBackgroundColors;
        LvtkHoverColors hoverTextColors;
        LvtkHoverColors dropdownItemHoverBackgroundColors;

        LvtkHoverOpacity dialHoverOpacity;


        LvtkStyle::ptr buttonDialogStyle;
        LvtkStyle::ptr buttonDialogPrimaryStyle;
        LvtkStyle::ptr buttonDialogSecondaryStyle;
        LvtkStyle::ptr buttonBorderStyle;
        LvtkStyle::ptr buttonBorderDefaultStyle;
        LvtkStyle::ptr buttonBorderPrimaryStyle;
        LvtkStyle::ptr buttonBorderSecondaryStyle;

        LvtkStyle::ptr imageButtonStyle;
        LvtkStyle::ptr imageButtonPrimaryStyle;
        LvtkStyle::ptr imageButtonSecondaryStyle;

        LvtkStyle::ptr buttonDisableStyle;



        LvtkStyle::ptr editBoxUnderlineStyle;
        LvtkStyle::ptr editBoxFrameStyle;
        LvtkStyle::ptr editBoxErrorStyle;
        LvtkStyle::ptr numericEditBoxStyle;

        LvtkStyle::ptr dropdownUnderlineStyle;
        LvtkStyle::ptr dropdownItemContainerStyle;
        LvtkStyle::ptr dropdownItemStyle;

        LvtkStyle::ptr dialStyle;
        LvtkStyle::ptr toggleButtonStyle;
        LvtkStyle::ptr toggleTrackStyle;
        LvtkStyle::ptr toggleThumbStyle;
        LvtkColor toggleButtonOffThumbColor;
        LvtkColor toggleButtonOffTrackColor;
        LvtkDropShadow toggleThumbDropShadow;
        LvtkDropShadow toggleTrackDropShadow;

        LvtkDropShadow menuDropShadow;

        LvtkDropShadow dialDropShadow;

        LvtkStyle::ptr vuStyle;
        LvtkStyle::ptr stereoVuStyle;
        LvtkStyle::ptr dbVuStyle;
        LvtkStyle::ptr stereoDbVuStyle;
        
        LvtkVuSettings vuSettings;
        LvtkVuSettings dbVuSettings;



        LvtkStyle::ptr lampStyle;

        LvtkStyle::ptr plotStyle;

        LvtkStyle::ptr statusTextStyle;

        LvtkStyle::ptr portGroupStyle;

        LvtkHoverColors scrollbarThumbColors;
        LvtkHoverColors scrollbarTrackColors;
        LvtkStyle::ptr horizontalScrollbarStyle;
        LvtkStyle::ptr collapsedHorizontalScrollbarStyle;
        LvtkStyle::ptr verticalScrollbarStyle;
        LvtkStyle::ptr collapsedVerticalScrollbarStyle;

        LvtkStyle::ptr indefiniteProgressStyle;

        // A place to store theme-related colors for custom controls. 
        std::map<std::string,LvtkColor> customColors;
        // A place to store theme-related styles for custom controls. 
        std::map<std::string,LvtkStyle::ptr> customStyles;
        // A place to store theme-related user daa for custom controls. 
        std::map<std::string,LvtkUserData::ptr> customUserData;
    };

} // namespace
