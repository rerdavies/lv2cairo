// MIT License
//
// Copyright (c) 2018 Yuki Koyama, 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Portions of this code ported from https://github.com/yuki-koyama/color-util

#include "lv2c/Lv2cCieColors.hpp"
#include <cassert>

using namespace lvtk;

Lv2cCieXyz lvtk::D65Illuminant{95.0489f, 100.0f, 108.8840f};
Lv2cCieXyz lvtk::D50Illuminant{96.4212f, 100.0f, 82.5188f};

Lv2cCieXyz::Lv2cCieXyz(float x, float y, float z)
    : x(x), y(y), z(z)
{
}

Lv2cCieXyz::Lv2cCieXyz(const Lv2cLinearColor &color)
{
    double a = color.a;
    double r, g, b;
    if (a == 0)
    {
        r = g = b = 0;
    }
    else
    {
        float invA = 1.0f / a;
        r = color.r * invA;
        g = color.g * invA;
        b = color.b * invA;
    }
    // BT.709-2 luma coefficientsm D65
    x = 100 * (r * 0.4124 + g * 0.3576 + b * 0.1805);
    y = 100 * (r * 0.2126 + g * 0.7152 + b * 0.0722);
    z = 100 * (r * 0.0193 + g * 0.1192 + b * 0.9505);

    this->alpha = color.a;
}

Lv2cLinearColor Lv2cCieXyz::ToLv2cLinearColor()
{
    float x = this->x * 0.01;
    float y = this->y * 0.01;
    float z = this->z * 0.01;
    float a = this->alpha;

    // BT.709-2 sRGB luma coefficiences, D65.
    float r = 3.2406 * x - 1.5372 * y - 0.4986 * z;
    float g = -0.9689 * x +1.8758 * y + 0.0415 * z;
    float b = 0.0557 * x - 0.2040 * y + 1.0570 * z;

    return Lv2cLinearColor(r * a, g * a, b * a, a);
}

Lv2cColor Lv2cCieXyz::ToLv2cColor()
{
    return Lv2cColor(ToLv2cLinearColor());
}

Lv2cCieXyz::Lv2cCieXyz(const Lv2cColor &color)
    : Lv2cCieXyz(Lv2cLinearColor(color))
{
}

static constexpr double delta = 6.0 / 29.0;
static constexpr double deltaSquared = delta * delta;
static constexpr double deltaCubed = delta * delta * delta;

static inline double f(double t)
{
    if (t < deltaCubed)
    {
        return t * (1 / (3 * deltaSquared) + 4.0 / 29.0);
    }
    else
    {
        double result = std::pow(t, 1.0 / 3.0);
        return result;
    }
}

Lv2cCieLab::Lv2cCieLab(
    const Lv2cCieXyz &color,
    const Lv2cCieXyz &illuminant)
{
    this->alpha = color.alpha;
    double fy = f(color.y / illuminant.y);
    this->L = 116 * fy - 16.0;
    this->a = 500 * (f(color.x / illuminant.x) - fy);
    this->b = 200 * (fy - f(color.z / illuminant.z));
}

static inline double invF(double t)
{
    if (t > delta)
    {
        return t * t * t;
    }
    else
    {
        return 3 * deltaSquared * (t - (4.0 / 29.0));
    }
}

Lv2cCieXyz::Lv2cCieXyz(const Lv2cCieLab &color, const Lv2cCieXyz &illuminant)
{
    this->alpha = color.alpha;
    this->x = illuminant.x * invF((color.L + 16) / 116 + color.a / 500);
    this->y = illuminant.y * invF((color.L + 16) / 116);
    this->z = illuminant.z * invF((color.L + 16) / 116 - color.b / 200);
}

Lv2cCieLCh::Lv2cCieLCh(const Lv2cCieLab &color)
{
    this->alpha = color.alpha;
    L = color.L;
    C = std::sqrt(color.a * color.a + color.b * color.b);
    if (C == 0)
    {
        h = 0;
    }
    else
    {
        h = atan2(color.b, color.a);
    }
}

Lv2cCieLab::Lv2cCieLab(
    const Lv2cCieLCh &color,
    const Lv2cCieXyz &illuminant)
{
    this->alpha = color.alpha;
    this->L = color.L;
    double angle = color.h;
    this->a = std::sin(angle) * color.C;
    this->b = std::cos(angle) * color.C;
}

Lv2cCieLCh::Lv2cCieLCh(
    const Lv2cColor &color,
    const Lv2cCieXyz &illuminant)
    : Lv2cCieLCh(
          Lv2cCieLab(
              Lv2cCieXyz(color),
              illuminant))
{
}


Lv2cColor Lv2cCieLab::ToLv2cColor(const Lv2cCieXyz& illuminant) const
{
    return Lv2cCieXyz(*this,illuminant).ToLv2cColor();
}

Lv2cColor Lv2cCieLCh::ToLv2cColor(const Lv2cCieXyz &illuminant)
{
    return Lv2cCieXyz(
               Lv2cCieLab(*this, illuminant))
        .ToLv2cColor();
}


/////////////////////////////////////////////////////////////////////////////////



static constexpr double epsilon = 1e-10;

static inline double my_atan(double y, double x)
{
    const double value = std::atan2(y, x) * 180.0 / M_PI;
    return (value < 0.0) ? value + 360.0 : value;
}

static inline double my_sin(double x) { return std::sin(x * M_PI / 180.0); }

static inline double my_cos(double x) { return std::cos(x * M_PI / 180.0); }

static inline double get_h_prime(double a_prime, double b)
{
    const bool a_prime_and_b_are_zeros = (std::abs(a_prime) < epsilon) && (std::abs(b) < epsilon);
    return a_prime_and_b_are_zeros ? 0.0 : my_atan(b, a_prime);
}

static inline double get_delta_h_prime(double C_1_prime, double C_2_prime, double h_1_prime, double h_2_prime)
{
    if (C_1_prime * C_2_prime < epsilon)
    {
        return 0.0;
    }

    const double diff = h_2_prime - h_1_prime;

    if (std::abs(diff) <= 180.0)
    {
        return diff;
    }
    else if (diff > 180.0)
    {
        return diff - 360.0;
    }
    else
    {
        return diff + 360.0;
    }
}
static inline double get_h_prime_bar(double C_1_prime, double C_2_prime, double h_1_prime, double h_2_prime)
{
    if (C_1_prime * C_2_prime < epsilon)
    {
        return h_1_prime + h_2_prime;
    }

    const double dist = std::abs(h_1_prime - h_2_prime);
    const double sum  = h_1_prime + h_2_prime;

    if (dist <= 180.0)
    {
        return 0.5 * sum;
    }
    else if (sum < 360.0)
    {
        return 0.5 * (sum + 360.0);
    }
    else
    {
        return 0.5 * (sum - 360.0);
    }
}


Lv2cCieLab::Lv2cCieLab(const Lv2cColor&color, const Lv2cCieXyz&illuminant)
:Lv2cCieLab(
    Lv2cCieXyz(color),
    illuminant)
{

}


double lvtk::CieColorDifference(const Lv2cColor& color_1, const Lv2cColor& color_2)
{
    return CieColorDifference(Lv2cCieLab(color_1),Lv2cCieLab(color_2));
}

double lvtk::CieColorDifference(const Lv2cCieLab& color_1, const Lv2cCieLab& color_2)
{
    // Copyright (c) 2018 Yuki Koyama
    //
    // Ported from https://github.com/yuki-koyama/color-util

    const double L_1 = color_1.L;
    const double a_1 = color_1.a;
    const double b_1 = color_1.b;
    const double L_2 = color_2.L;
    const double a_2 = color_2.a;
    const double b_2 = color_2.b;

    // Step 1

    const double C_1_ab = std::sqrt(a_1 * a_1 + b_1 * b_1);
    const double C_2_ab = std::sqrt(a_2 * a_2 + b_2 * b_2);
    const double C_ab_bar = 0.5 * (C_1_ab + C_2_ab);
    const double G =
        0.5 * (1.0 - std::sqrt(std::pow(C_ab_bar, 7.0) / (std::pow(C_ab_bar, 7.0) + std::pow(25.0, 7.0))));
    const double a_1_prime = (1.0 + G) * a_1;
    const double a_2_prime = (1.0 + G) * a_2;
    const double C_1_prime = std::sqrt(a_1_prime * a_1_prime + b_1 * b_1);
    const double C_2_prime = std::sqrt(a_2_prime * a_2_prime + b_2 * b_2);
    const double h_1_prime = get_h_prime(a_1_prime, b_1);
    const double h_2_prime = get_h_prime(a_2_prime, b_2);

    // Step 2

    const double delta_L_prime = L_2 - L_1;
    const double delta_C_prime = C_2_prime - C_1_prime;
    const double delta_h_prime = get_delta_h_prime(C_1_prime, C_2_prime, h_1_prime, h_2_prime);
    const double delta_H_prime = 2.0 * std::sqrt(C_1_prime * C_2_prime) * my_sin(0.5 * delta_h_prime);

    // Step 3

    const double L_prime_bar = 0.5 * (L_1 + L_2);
    const double C_prime_bar = 0.5 * (C_1_prime + C_2_prime);
    const double h_prime_bar = get_h_prime_bar(C_1_prime, C_2_prime, h_1_prime, h_2_prime);

    const double T = 1.0 - 0.17 * my_cos(h_prime_bar - 30.0) + 0.24 * my_cos(2.0 * h_prime_bar) +
                     0.32 * my_cos(3.0 * h_prime_bar + 6.0) - 0.20 * my_cos(4.0 * h_prime_bar - 63.0);

    const double delta_theta = 30.0 * std::exp(-((h_prime_bar - 275) / 25.0) * ((h_prime_bar - 275) / 25.0));

    const double R_C =
        2.0 * std::sqrt(std::pow(C_prime_bar, 7.0) / (std::pow(C_prime_bar, 7.0) + std::pow(25.0, 7.0)));
    const double S_L = 1.0 + (0.015 * (L_prime_bar - 50.0) * (L_prime_bar - 50.0)) /
                                 std::sqrt(20.0 + (L_prime_bar - 50.0) * (L_prime_bar - 50.0));
    const double S_C = 1.0 + 0.045 * C_prime_bar;
    const double S_H = 1.0 + 0.015 * C_prime_bar * T;
    const double R_T = -my_sin(2.0 * delta_theta) * R_C;

    constexpr double k_L = 1.0;
    constexpr double k_C = 1.0;
    constexpr double k_H = 1.0;

    const double delta_L = delta_L_prime / (k_L * S_L);
    const double delta_C = delta_C_prime / (k_C * S_C);
    const double delta_H = delta_H_prime / (k_H * S_H);

    const double delta_E_squared =
        delta_L * delta_L + delta_C * delta_C + delta_H * delta_H + R_T * delta_C * delta_H;

    return std::sqrt(delta_E_squared);
}


