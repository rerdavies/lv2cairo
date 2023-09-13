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

#pragma once
#include "LvtkTypes.hpp"
#include <memory>
#include <string>

typedef struct _RsvgHandle RsvgHandle;

namespace lvtk
{
    class LvtkSvg
    {
    public:
        using ptr = std::shared_ptr<LvtkSvg>;
        static ptr Create() { return std::make_shared<LvtkSvg>(); }

        LvtkSvg() {}

        LvtkSvg(const LvtkSvg&other);
        LvtkSvg(LvtkSvg&&other);
        LvtkSvg&operator=(const LvtkSvg&other);
        LvtkSvg&operator=(LvtkSvg&&other);
        virtual ~LvtkSvg() noexcept;

        RsvgHandle*get() { return handle; }
        const RsvgHandle*get() const { return handle; }

        void load(const std::string&filename);
        LvtkSize intrinsic_size() const;

        void render(LvtkDrawingContext&context,const LvtkRectangle &viewport);
    private:
        LvtkSize calculate_intrinsic_size();
        void clear();
        void set(RsvgHandle *value);
        RsvgHandle *handle = nullptr;
        LvtkSize intrinsicSize {24,24};
    };
} // namespace