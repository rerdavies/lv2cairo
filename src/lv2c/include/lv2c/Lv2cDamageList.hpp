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

#include "Lv2cTypes.hpp"
#include <vector>
#include <memory>

namespace lv2c {

    class Lv2cDamageList {
    public:
        void Invalidate(const Lv2cRectangle &rectangle);
        void ExposeRect(int64_t x, int64_t y, int64_t width, int64_t height);

        std::vector<Lv2cRectangle> GetDamageList();

        void SetSize(int64_t width, int64_t height);
        int64_t Width() const;
        int64_t Height() const;
    private:

        class DamageRect {
        public:
            DamageRect() { }
            DamageRect(int64_t left, int64_t right, int64_t top, int64_t bottom) : left(left),right(right),top(top),bottom(bottom) {}

            static DamageRect intersect(const DamageRect &r0, const DamageRect&r1);

            bool isEmpty() { return right <= left || bottom <= top; }

            int64_t left, right,top,bottom;
        };
        void ExposeRect(DamageRect rect);


        DamageRect bounds;

        struct DamageLine {
            using ptr = std::unique_ptr<DamageLine>;

            DamageLine(const DamageRect &rect)
            : top(rect.top),
              bottom(rect.bottom)
            {   
                points.push_back(rect.left);
                points.push_back(rect.right);
            }

            void addRange(int64_t left, int64_t right);
            bool contains(int64_t left, int64_t right);

            int64_t top;
            int64_t bottom;
            std::vector<int64_t> points;
        };

        static bool canMerge(const DamageLine*line1,const DamageLine*line2);

        std::vector<DamageLine::ptr> damageLines;
    };

} // namespace