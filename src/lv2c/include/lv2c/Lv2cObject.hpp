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

namespace lvtk 
{
    class Lv2cObject {
    public:
        virtual ~Lv2cObject();
        void  CheckValid() const;
    private:
        static constexpr uint64_t MAGIC_GUARD = 0x134AD34BED341990;
        uint64_t useAfterFreeGuard = MAGIC_GUARD;

    };


    ////////////////////////////////////
    inline void  Lv2cObject::CheckValid() const {
        if (useAfterFreeGuard != MAGIC_GUARD)
        {
            throw std::runtime_error("Use after free.");
        }
    }

    inline Lv2cObject::~Lv2cObject()
    {
        CheckValid();
        useAfterFreeGuard = 0xBAADF00DBAADF00D;
    }

}