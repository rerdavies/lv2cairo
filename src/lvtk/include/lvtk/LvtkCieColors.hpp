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

#include "lvtk/LvtkTypes.hpp"


namespace lvtk
{

    class LvtkCieLab; // forward declaration.
    class LvtkCieLCh; // forward declaration.
    class LvtkCieXyz;
    extern LvtkCieXyz D65Illuminant;
    extern LvtkCieXyz D50Illuminant;


    /// @brief Cie XYZ 2 degree,, with D65 illuminant.
    /// Alpha is linear; colors are not pre-multiplied.
    class LvtkCieXyz {
    public:
        explicit LvtkCieXyz(const LvtkLinearColor &color);
        explicit LvtkCieXyz(const LvtkColor &color);

        explicit LvtkCieXyz(const LvtkCieLab &color,const LvtkCieXyz&illuminant = D65Illuminant);

        explicit LvtkCieXyz(float x, float y, float z);

        LvtkLinearColor ToLvtkLinearColor();
        LvtkColor ToLvtkColor();


        float x,y,z;
        float alpha; // linear alpha.
    };

    class LvtkCieLab {
    public:
        explicit LvtkCieLab(const LvtkCieXyz&color, const LvtkCieXyz&illuminant = D65Illuminant);
        explicit LvtkCieLab(const LvtkColor&color, const LvtkCieXyz&illuminant = D65Illuminant);
        explicit LvtkCieLab(double L, double a, double b, double alpha=1.0) : L(L),a(a),b(b),alpha(alpha) {}

        explicit LvtkCieLab(const LvtkCieLCh&color, const LvtkCieXyz&illuminant = D65Illuminant);

        LvtkColor ToLvtkColor(const LvtkCieXyz&illuminant = D65Illuminant) const;

        double L,a,b, alpha;
    };

    class LvtkCieLCh  {
    public:
        explicit LvtkCieLCh(const LvtkCieLab &color);
        explicit LvtkCieLCh(const LvtkColor &color, const LvtkCieXyz&illuminant = D65Illuminant);

        LvtkColor ToLvtkColor(const LvtkCieXyz&illuminant = D65Illuminant);

        double L, C,h, alpha;
    };

    /// @brief CIEDE2000 color difference.
    double CieColorDifference(const LvtkCieLab& color_1, const LvtkCieLab& color_2);
    double CieColorDifference(const LvtkColor& color_1, const LvtkColor& color_2);
    

}