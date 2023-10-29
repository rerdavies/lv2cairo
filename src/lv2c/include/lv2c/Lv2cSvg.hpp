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
#include <memory>
#include <string>

typedef struct _RsvgHandle RsvgHandle;

namespace lvtk
{
    class Lv2cSvg
    {
    public:
        using ptr = std::shared_ptr<Lv2cSvg>;
        static ptr Create() { return std::make_shared<Lv2cSvg>(); }

        Lv2cSvg() {}

        Lv2cSvg(const Lv2cSvg&other);
        Lv2cSvg(Lv2cSvg&&other);
        Lv2cSvg&operator=(const Lv2cSvg&other);
        Lv2cSvg&operator=(Lv2cSvg&&other);
        virtual ~Lv2cSvg() noexcept;

        RsvgHandle*get() { return handle; }
        const RsvgHandle*get() const { return handle; }

        void load(const std::string&filename);
        Lv2cSize intrinsic_size() const;

        void render(Lv2cDrawingContext&context,const Lv2cRectangle &viewport);
    private:
        Lv2cSize calculate_intrinsic_size();
        void clear();
        void set(RsvgHandle *value);
        RsvgHandle *handle = nullptr;
        Lv2cSize intrinsicSize {24,24};
    };
} // namespace