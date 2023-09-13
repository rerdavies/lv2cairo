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

#include <catch2/catch.hpp>
#include "lvtk/LvtkTypes.hpp"
#include <cmath>
#include <iostream>

using namespace lvtk;


static bool ApproxEqual(double v1, double v2)
{
    return std::abs(v1-v2) < 1E-5;
}

static bool ColorEqual(const LvtkColor& v1, const LvtkColor& v2)
{
    if (std::abs(v1.R()-v2.R()) > 1E-5) 
        return true;
    if (std::abs(v1.G()-v2.G()) > 1E-5) 
        return true;
    if (std::abs(v1.B()-v2.B()) > 1E-5) 
        return true;
    return ApproxEqual(v1.A(),v2.A());
}


static void TestLvtkColorBlending()
{
    {
        LvtkColor c0 { 1,0.5,0.25,1};

        for (double blend = 0; blend <= 1.0; blend += 1.0/8)
        {
            LvtkColor result = LvtkColor::Blend(blend,c0,c0);
            REQUIRE(result == c0);
        }
    }
    {
        LvtkColor c0 { 1,0.5,0.25,0.5};

        for (double blend = 0; blend <= 1.0; blend += 1.0/8)
        {
            LvtkColor result = LvtkColor::Blend(blend,c0,c0);
            REQUIRE(true ==  ColorEqual(result,c0));
        }
    }
    {
        LvtkColor c0 { 1,0.5,0.25,0};
        LvtkColor c1 { 1,0.5,0.25,1};

        for (double blend = 0; blend <= 1.0; blend += 1.0/8)
        {
            LvtkColor result = LvtkColor::Blend(blend,c0,c1);
            REQUIRE(true ==  ApproxEqual(result.A(),blend));
            REQUIRE(true ==  ApproxEqual(result.R(),c1.R()));
            REQUIRE(true ==  ApproxEqual(result.G(),c1.G()));
            REQUIRE(true ==  ApproxEqual(result.B(),c1.B()));
        }
    }
    {
        LvtkColor c0 { 0,0.0,0.0,0};
        LvtkColor c1 { 1,0.5,0.25,1};

        for (double blend = 0; blend <= 1.0; blend += 1.0/8)
        {
            if (blend != 0)
            {
                LvtkColor result = LvtkColor::Blend(blend,c0,c1);
                REQUIRE(true ==  ApproxEqual(result.A(),blend));
                REQUIRE(true ==  ApproxEqual(result.R(),c1.R()));
                REQUIRE(true ==  ApproxEqual(result.G(),c1.G()));
                REQUIRE(true ==  ApproxEqual(result.B(),c1.B()));
            }
        }
    }
    {
        LvtkColor c0 { 0.1,.2,0.3,0.25};
        LvtkColor c1 { 0.4,0.5,0.6,0.75};

        REQUIRE(true ==  ColorEqual(c0, LvtkColor::Blend(0,c0,c1)));
        REQUIRE(true ==  ColorEqual(c1, LvtkColor::Blend(1,c0,c1)));
    }
}

using namespace lvtk::implementation;

static void TestLvtkLinearColor()
{
    double maxError = 0;
    // test round-trip errors for 
    for (int i = 0; i < 256; ++i)
    {
        double value = SrgbToI((uint8_t)i);
        double roundTrip = IToSrgb(value);
        double error = std::abs(roundTrip-i);
        REQUIRE(error < 1);
        if (error > maxError)
        {
            maxError = error;
        }
        REQUIRE(i == (uint8_t)(roundTrip+0.5));
    }



    std::cout  << "Max rountrip error for SrgbToI/IToSrgb: " << maxError << std::endl;
}
TEST_CASE("LvtkColor blend test", "[color_blend]")
{
    TestLvtkColorBlending();
    TestLvtkLinearColor();
}


