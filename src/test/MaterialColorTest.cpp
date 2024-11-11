#include "CatchTest.hpp"
#include "lv2c/Lv2cTypes.hpp"
#include <cmath>
#include <iostream>
#include "lv2c/Lv2cCieColors.hpp"

using namespace lv2c;
using namespace std;


static void TestMaterialPalette(
    const std::string name,
    const std::vector<std::string> &mainColors,
    const std::vector<std::string> accentColors)
{
    cout << "--- " << name << " ---" << endl;
    static int variantIndexes[]{
        50, 100, 200, 300, 400, 500, 600, 700, 800, 900};
    REQUIRE(mainColors.size() == sizeof(variantIndexes) / sizeof(variantIndexes[0]));

    for (size_t i = 0; i < mainColors.size(); ++i)
    {
        Lv2cColor cc {mainColors[i]};
        Lv2cLinearColor lcc {cc};

        Lv2cCieLCh  lch (cc);
        Lv2cLinearColor lc { cc};

        cout << variantIndexes[i] << " " << mainColors[i]
             << " RGB: " << cc.R()
             << " " << cc.G()
             << " " << cc.B()

             << " LRGB: " << lc.r
             << " " << lc.g
             << " " << lc.b

             << " LCh: " << lch.L
             << " " <<lch.C
             << " " << lch.h
             << endl;
    }
}

static bool approxEqual(float v1, float v2)
{
    float error= std::abs(v1-v2);
    return error < 1.0E5;
}
static void CieRoundTripTest()
{
    std::vector<std::string> colors {
        "#FFFFFF","#000000","#800000","#008000","#000080"
        ,"#00008080","#0309124C","#000001"
    };

    for (auto&color: colors)
    {
        Lv2cColor cc { color};
        Lv2cCieLCh lch { cc};

        Lv2cColor ccOut = lch.ToLv2cColor();

        CHECK(approxEqual(ccOut.R(),cc.R()));
        CHECK(approxEqual(ccOut.G(),cc.G()));
        CHECK(approxEqual(ccOut.B(),cc.B()));
        CHECK(approxEqual(ccOut.A(),cc.A()));
    }
}
TEST_CASE("Material Colortest", "[material_color_blend]")
{


    {
        // color difference from Lab
        Lv2cCieLab c1 { 50.0, 2.6772,-79.7751};
        Lv2cCieLab c2 { 50.0, 0.0,-82.7485};

        double result = CieColorDifference(c1,c2);
        REQUIRE (std::abs(result-2.0425) < 1E-2);
    }
    {
        // color difference from Lab
        Lv2cCieLab c1 { 50.0, 2.5,0};
        Lv2cCieLab c2 { 73.0, 25.0,-18};

        double result = CieColorDifference(c1,c2);
        REQUIRE (std::abs(result-27.1492) < 1E-2);
    }
    {
        // color difference.

        Lv2cColor c1 { 200/255.0f,100/255.0f,20/255.0f };

        Lv2cCieXyz xyz1 { c1};
        REQUIRE ((
            std::abs(xyz1.x - 28.51) < 0.1
            && std::abs(xyz1.y - 21.45) < 0.1
            && std::abs(xyz1.z-3.30) < 0.01
        ));
        Lv2cColor c1RoundTrip = xyz1.ToLv2cColor();

        REQUIRE ((
            std::abs(c1.R()-c1RoundTrip.R()) < 0.1
            && std::abs(c1.G()-c1RoundTrip.G()) < 0.1
            && std::abs(c1.B()-c1RoundTrip.B()) < 0.1
        ));

        Lv2cCieLab lab1 { c1};

        Lv2cColor c2 { 100/255.0f,200/255.0f,50/255.0f };

        REQUIRE((
            std::abs(lab1.L - 53.44) < 0.1
            && std::abs(lab1.a-35.39) < 0.1
            && std::abs(lab1.b-57.35) < 0.1
            ));

        double difference = CieColorDifference(c1,c2);
        REQUIRE( std::abs(difference-53.5541) < 1E-4);
    }

    TestMaterialPalette(
        "Purple",
        {
            "#F3E5F5",
            "#E1BEE7",
            "#CE93D8",
            "#Ba68C8",
            "#AB47BC",
            "#9C27B0",
            "#8E24AA",
            "#7B1FA2",
            "#6A1B91",
            "#4A148C",
        },
        {"#EA80FC",
         "#E040FB",
         "#D500F9",
         "#AA00FF"});

    TestMaterialPalette(
        "Red",
        {
            "#FFEBEE",
            "#FFFDD2",
            "#EF9A9A",
            "#E57373",
            "#EF5350",
            "#F44336",
            "#E53935",
            "#D32F2F",
            "#C62828",
            "#B71C1C",
        },
        {"#FF8A80",
         "#FF5252",
         "#FF1744",
         "#D50000"});
    CieRoundTripTest();

}
