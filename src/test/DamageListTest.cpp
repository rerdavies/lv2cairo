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

#include "lvtk/LvtkDamageList.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdlib>
using namespace std;
using namespace lvtk;


void Check2RectComposition(const LvtkRectangle &c, const LvtkRectangle &c2, size_t expectedRects)
{
    LvtkDamageList list;
    list.SetSize(1000, 1000);
    list.GetDamageList();

    list.Invalidate(c);
    list.Invalidate(c2);

    auto damageResults = list.GetDamageList();

    REQUIRE(damageResults.size() == expectedRects);

    double damageArea = 0;
    for (auto &rect : damageResults)
    {
        damageArea += rect.Area();
    }

    double expectedArea = c.Area() + c2.Area() - c.Intersect(c2).Area();

    REQUIRE(expectedArea == damageArea);
}

void ColumnTests()
{

    Check2RectComposition(LvtkRectangle(1, 0, 3, 1), LvtkRectangle(3, 0, 3, 1), 1);
    Check2RectComposition(LvtkRectangle(1, 0, 3, 1), LvtkRectangle(1, 0, 3, 1), 1);
    Check2RectComposition(LvtkRectangle(3, 0, 3, 1), LvtkRectangle(1, 0, 3, 1), 1);
    Check2RectComposition(LvtkRectangle(3, 0, 3, 1), LvtkRectangle(6, 0, 3, 1), 1);
    Check2RectComposition(LvtkRectangle(3, 0, 3, 1), LvtkRectangle(3, 0, 6, 1), 1);
    Check2RectComposition(LvtkRectangle(3, 0, 3, 1), LvtkRectangle(4, 0, 1, 1), 1);
    Check2RectComposition(LvtkRectangle(2, 0, 3, 1), LvtkRectangle(1, 0, 7, 1), 1);
    Check2RectComposition(LvtkRectangle(2, 0, 3, 1), LvtkRectangle(1, 0, 7, 1), 1);

    Check2RectComposition(LvtkRectangle(2, 0, 3, 1), LvtkRectangle(8, 0, 7, 1), 2);

    Check2RectComposition(LvtkRectangle(2, 0, 3, 1), LvtkRectangle(8, 0, 7, 1), 2);

    Check2RectComposition(LvtkRectangle(9, 0, 3, 1), LvtkRectangle(1, 0, 1, 1), 2);
}

// Perform tests on a 3x3 area. 
void TicTacToeCheck(size_t expectedRectangles, double expectedArea, const std::vector<LvtkRectangle> &rectangles)
{
    LvtkDamageList list;
    list.SetSize(3, 3);
    list.GetDamageList(); // clear initial damage.

    for (const auto &rectangle : rectangles)
    {
        list.Invalidate(rectangle);
    }

    auto damageResults = list.GetDamageList();

    REQUIRE(damageResults.size() == expectedRectangles);

    double damageArea = 0;
    for (auto &rect : damageResults)
    {
        damageArea += rect.Area();
    }

    REQUIRE(expectedArea == damageArea);
}
void RowTests()
{

    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(0, 1, 5, 1), 3);
    Check2RectComposition(LvtkRectangle(0, 1, 5, 1), LvtkRectangle(1, 0, 1, 5), 3);
    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(0, 0, 5, 1), 2);
    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(2, 0, 5, 1), 2);
    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(0, 0, 1, 1), 2);
    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(0, 0, 2, 1), 2);
    Check2RectComposition(LvtkRectangle(1, 0, 1, 5), LvtkRectangle(0, 2, 1, 1), 3);
}


std::vector<LvtkRectangle> shuffleRects(const std::vector<LvtkRectangle> &rectangles)
{
    std::vector<LvtkRectangle> result = rectangles;
    for (size_t i = 0; i < result.size(); ++i)
    {
        int rnd = std::rand() % (result.size()-i);

        std::swap(result[i],result[i+rnd]);
    }

    return result;
}

void TicTacToeShuffleCheck(size_t expectedRectangles, double expectedArea, const std::vector<LvtkRectangle> &rectangles)
{
    TicTacToeCheck(expectedRectangles, expectedArea, rectangles);

    for (int i = 0; i < 40; ++i)
    {
        std::vector<LvtkRectangle> shuffledRects = shuffleRects(rectangles);
        TicTacToeCheck(expectedRectangles, expectedArea, shuffledRects);
    }
}


void TickTacToeTest()
{
    // Results should be the same no matter which order.
    // Do random permutations of rectangles in order to provde better coverage of cases.
    TicTacToeShuffleCheck(1, 4, {LvtkRectangle(0, 0, 2, 1), LvtkRectangle(0, 0, 1, 2), LvtkRectangle(1, 1, 1, 1)});
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(-1, 0, 5, 1), // donut
                                    LvtkRectangle(0, 1, 1, 1), LvtkRectangle(2, 1, 1, 1), LvtkRectangle(0, 2, 5, 1),

                                    LvtkRectangle(1, 1, 1, 1) // hole.
                                });
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(0, 0, 2, 1),
                                    LvtkRectangle(1, 0, 2, 1),
                                    LvtkRectangle(0, 1, 1, 1),
                                    LvtkRectangle(2, 1, 1, 1),
                                    LvtkRectangle(0, 2, 3, 1),
                                    LvtkRectangle(1, 1, 1, 1),
                                });

    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(1, 0, 1, 3),
                                    LvtkRectangle(0, 1, 3, 1),

                                    LvtkRectangle(0, 0, 1, 1),
                                    LvtkRectangle(2, 0, 1, 1),

                                    LvtkRectangle(0, 2, 1, 1),
                                    LvtkRectangle(2, 2, 1, 1),
                                });
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(1, 0, 1, 3),
                                    LvtkRectangle(0, 1, 3, 1),

                                    LvtkRectangle(0, 0, 2, 1),
                                    LvtkRectangle(2, 0, 1, 1),

                                    LvtkRectangle(0, 2, 1, 1),
                                    LvtkRectangle(1, 2, 2, 1),
                                });

    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(2, 0, 1, 3),
                                    LvtkRectangle(0, 1, 3, 1),

                                    LvtkRectangle(0, 0, 1, 1),
                                    LvtkRectangle(1, 0, 1, 1),

                                    LvtkRectangle(0, 2, 1, 1),
                                    LvtkRectangle(1, 2, 1, 1),
                                });
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(2, 0, 1, 3),
                                    LvtkRectangle(0, 1, 3, 1),

                                    LvtkRectangle(0, 0, 1, 1),
                                    LvtkRectangle(1, 0, 1, 1),

                                    LvtkRectangle(0, 2, 3, 1),
                                });
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(2, 0, 1, 3),
                                    LvtkRectangle(0, 1, 3, 1),

                                    LvtkRectangle(0, 0, 1, 1),
                                    LvtkRectangle(1, 0, 2, 1),

                                    LvtkRectangle(0, 2, 2, 1),
                                    LvtkRectangle(1, 2, 1, 1),
                                });
    TicTacToeShuffleCheck(1, 9, {
                                    LvtkRectangle(0, 0, 3, 3),
                                    LvtkRectangle(0, 1, 3, 1),
                                    LvtkRectangle(0, 0, 1, 1),
                                    LvtkRectangle(1, 0, 2, 1),

                                    LvtkRectangle(0, 2, 2, 1),
                                    LvtkRectangle(1, 2, 1, 1),
                                });

}

TEST_CASE( "DamageList Test", "[damage_list]" ) {
     RowTests();
    ColumnTests();
}
// int main(int argc, char **argv)
// {
//     std::srand(1);

//     try
//     {
//         RowTests();
//         ColumnTests();
//     }
//     catch (const std::exception &e)
//     {
//         cout << "Test failed: " << e.what() << endl;
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }