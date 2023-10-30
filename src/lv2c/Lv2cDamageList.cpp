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

#include "lv2c/Lv2cDamageList.hpp"
#include <cmath>

using namespace lv2c;


int64_t Lv2cDamageList::Width() const
{
    return this->bounds.right-this->bounds.left;
}
int64_t Lv2cDamageList::Height() const
{
    return this->bounds.bottom-this->bounds.top;
}

void Lv2cDamageList::SetSize(int64_t width, int64_t height)
{
    this->bounds = DamageRect{0,width,0,height};
    damageLines.resize(0);
    Lv2cRectangle rc { 0,0,(double)width,(double)height};
    Invalidate(rc);
}

void Lv2cDamageList::ExposeRect(int64_t x, int64_t y, int64_t width, int64_t height)
{
    DamageRect rect {x,x+width,y,y+height};
    ExposeRect(rect);
}

void Lv2cDamageList::Invalidate(const Lv2cRectangle &rectangle)
{
    DamageRect rect { 
        (int64_t)std::floor(rectangle.Left()),
        (int64_t)std::ceil(rectangle.Right()),
        (int64_t)std::floor(rectangle.Top()),
        (int64_t)std::ceil(rectangle.Bottom())
    };
    ExposeRect(rect);
}
void Lv2cDamageList::ExposeRect(DamageRect rect)
{
    rect = DamageRect::intersect(rect,this->bounds);
    if (rect.isEmpty())
    {
        return;
    }

    size_t damageLine = 0;
    if (damageLines.size() == 0)
    {
        damageLines.push_back(std::make_unique<DamageLine>(rect));
        return;
    }
    while (damageLine < damageLines.size()) {
        auto & line = damageLines[damageLine];
        if (line->bottom < rect.top)
        {
            ++damageLine;
        } else {
            break;
        }
    }
    while (rect.bottom > rect.top)
    {
        if (damageLine >= damageLines.size())
        {
            damageLines.push_back(std::make_unique<DamageLine>(rect));
            break;
        }
        DamageLine*damageRow = damageLines[damageLine].get();

        if (rect.top < damageRow->top)
        {
            damageLines.insert(damageLines.begin()+damageLine, 
                std::make_unique<DamageLine>(
                    DamageRect {rect.left,rect.right,rect.top,damageRow->top}));
            rect.top = damageRow->top;
            ++damageLine;
        } else if (rect.top == damageRow->top)
        {
            if (rect.bottom >= damageRow->bottom) {
                damageRow->addRange(rect.left,rect.right);
                rect.top = damageRow->bottom;
                ++damageLine;
            } else {
                if (damageRow->contains(rect.left,rect.right))
                {
                    break;
                }
                auto newLine = std::make_unique<DamageLine>(*damageRow); // copy construct.
                newLine->bottom = rect.bottom;
                damageRow->top = rect.bottom;
                rect.top = rect.bottom;
                newLine->addRange(rect.left,rect.right);
                damageLines.insert(damageLines.begin()+damageLine,std::move(newLine));
                ++damageLine;
            }
        } else if (rect.top < damageRow->bottom)
        {
            auto newLine = std::make_unique<DamageLine>(*damageRow); // copy construct.
            newLine->bottom = rect.top;
            damageRow->top = rect.top;
            damageLines.insert(damageLines.begin()+damageLine,std::move(newLine));
            ++damageLine;
        } else {
            ++damageLine;
        }
    }
    // check for row merges

    size_t row = 0;

    while (row < damageLines.size()-1)
    {
        if (canMerge(damageLines[row].get(),damageLines[row+1].get()))
        {
            damageLines[row]->bottom = damageLines[row+1]->bottom;
            damageLines.erase(damageLines.begin()+row+1);
        } else {
            ++row;
        }
    }
}
std::vector<Lv2cRectangle> Lv2cDamageList::GetDamageList()
{
    std::vector<Lv2cRectangle> result;

    for (auto& damageLine: damageLines)
    {
        double top = (double)(damageLine->top);
        double bottom = (double)(damageLine->bottom);

        auto &points = damageLine->points;

        for (size_t i = 0; i < points.size(); i += 2)
        {
            result.push_back(
                Lv2cRectangle(
                    (double)(points[i]),
                    top,
                    (double)(points[i+1]-points[i]),
                    bottom-top)
                );
        }
    }
    damageLines.resize(0);
    return result;
}



Lv2cDamageList::DamageRect  Lv2cDamageList::DamageRect::intersect(const DamageRect &r0, const DamageRect&r1)
{
    int64_t left = std::max(r0.left,r1.left);
    int64_t right = std::min(r0.right,r1.right);
    if (right  <= left) {
        return DamageRect{0,0,0,0};
    }
    int64_t top = std::max(r0.top,r1.top);
    int64_t bottom = std::min(r0.bottom,r1.bottom);
    if (bottom <= top) {
        return DamageRect{0,0,0,0};
    }
    return DamageRect{left,right,top,bottom};
}



void Lv2cDamageList::DamageLine::addRange(int64_t left, int64_t right)
{
    auto & points = this->points;
    if (left == right) return;

    for (size_t i = 0; i < points.size(); i += 2)
    {
        int64_t ptLeft = points[i];
        int64_t ptRight = points[i+1];

        if (left < ptLeft)
        {
            if (right < ptLeft)
            {
                //    .... 
                //         ....
                points.insert(points.begin()+i, left);
                points.insert(points.begin()+(i+1), right);
                return;
            }
            
            if (right <= ptRight)
            {
                //  ........
                //    ......
                points[i] = left;
                return;
            } else {
                // point range completely included in [left,right)
                //    ...........
                //       .......
                points.erase(points.begin()+i,points.begin()+i+2);
                i -= 2;
                // and go again.
            }
        } else if (left <= ptRight)
        {
            //        ..
            //     .....
            if (right <= ptRight)
            {
                return;
            }
            //      .......... 
            //   ....     . ...

            left = ptLeft;
            points.erase(points.begin()+i,points.begin()+i+2);
            i -= 2;
            // and go again.
        }
    }
    points.push_back(left);
    points.push_back(right);

}

bool Lv2cDamageList::DamageLine::contains(int64_t left, int64_t right)
{
    auto & points = this->points;

    for (size_t i = 0; i < points.size(); i += 2)
    {
        int64_t ptLeft = points[i];
        int64_t ptRight = points[i+1];
        if (ptLeft > right) return false;
        if (ptLeft <= left && ptRight >= right) return true;
    }
    return false;
}


bool Lv2cDamageList::canMerge(const DamageLine*line1,const DamageLine*line2) 
{
    if  (line1->bottom != line2->top) return false;

    return line1->points == line2->points;
}
