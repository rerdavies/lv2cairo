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

#include "lv2c/Lv2cTypes.hpp"


namespace lv2c
{

    class Lv2cCieLab; // forward declaration.
    class Lv2cCieLCh; // forward declaration.
    class Lv2cCieXyz;
    extern Lv2cCieXyz D65Illuminant;
    extern Lv2cCieXyz D50Illuminant;


    /// @brief Cie XYZ 2 degree,, with D65 illuminant.
    /// Alpha is linear; colors are not pre-multiplied.
    class Lv2cCieXyz {
    public:
        explicit Lv2cCieXyz(const Lv2cLinearColor &color);
        explicit Lv2cCieXyz(const Lv2cColor &color);

        explicit Lv2cCieXyz(const Lv2cCieLab &color,const Lv2cCieXyz&illuminant = D65Illuminant);

        explicit Lv2cCieXyz(float x, float y, float z);

        Lv2cLinearColor ToLv2cLinearColor();
        Lv2cColor ToLv2cColor();


        float x,y,z;
        float alpha; // linear alpha.
    };

    class Lv2cCieLab {
    public:
        explicit Lv2cCieLab(const Lv2cCieXyz&color, const Lv2cCieXyz&illuminant = D65Illuminant);
        explicit Lv2cCieLab(const Lv2cColor&color, const Lv2cCieXyz&illuminant = D65Illuminant);
        explicit Lv2cCieLab(double L, double a, double b, double alpha=1.0) : L(L),a(a),b(b),alpha(alpha) {}

        explicit Lv2cCieLab(const Lv2cCieLCh&color, const Lv2cCieXyz&illuminant = D65Illuminant);

        Lv2cColor ToLv2cColor(const Lv2cCieXyz&illuminant = D65Illuminant) const;

        double L,a,b, alpha;
    };

    class Lv2cCieLCh  {
    public:
        explicit Lv2cCieLCh(const Lv2cCieLab &color);
        explicit Lv2cCieLCh(const Lv2cColor &color, const Lv2cCieXyz&illuminant = D65Illuminant);

        Lv2cColor ToLv2cColor(const Lv2cCieXyz&illuminant = D65Illuminant);

        double L, C,h, alpha;
    };

    /// @brief CIEDE2000 color difference.
    double CieColorDifference(const Lv2cCieLab& color_1, const Lv2cCieLab& color_2);
    double CieColorDifference(const Lv2cColor& color_1, const Lv2cColor& color_2);
    

}