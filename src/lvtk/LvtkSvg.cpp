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

#include "lvtk/LvtkSvg.hpp"
#include "lvtk/LvtkDrawingContext.hpp"
#include "lvtk/LvtkLog.hpp"
#include "librsvg/rsvg.h"
#include <stdexcept>
#include "ss.hpp"

using namespace lvtk;

LvtkSvg::~LvtkSvg() noexcept
{
    clear();
}

void LvtkSvg::set(RsvgHandle *value)
{
    this->handle = (RsvgHandle *)g_object_ref(value);
}
void LvtkSvg::clear()
{
    if (this->handle)
    {
        g_object_unref(this->handle);
        this->handle = nullptr;
    }
}
LvtkSvg::LvtkSvg(const LvtkSvg &other)
{
    set(const_cast<RsvgHandle *>(other.handle));
}
LvtkSvg::LvtkSvg(LvtkSvg &&other)
{
    this->handle = nullptr;
    std::swap(handle, other.handle);
}
LvtkSvg &LvtkSvg::operator=(const LvtkSvg &other)
{
    clear();
    set(const_cast<RsvgHandle *>(other.get()));
    return *this;
}
LvtkSvg &LvtkSvg::operator=(LvtkSvg &&other)
{
    std::swap(this->handle, other.handle);
    return *this;
}

void LvtkSvg::load(const std::string &filename)
{
    intrinsicSize = LvtkSize(24, 24);
    GError *error = nullptr;
    this->handle = rsvg_handle_new_from_file(
        filename.c_str(),
        &error);
    if (!this->handle)
    {
        throw std::runtime_error(SS(error->message << "(" << error->code << ")"));
    }
    intrinsicSize = calculate_intrinsic_size();
}

LvtkSize LvtkSvg::intrinsic_size() const
{
    return this->intrinsicSize;
}
LvtkSize LvtkSvg::calculate_intrinsic_size()
{

    constexpr double INCHES_PER_CM = 0.393701;
    constexpr double INCHES_PER_MM = 0.0393701;
    constexpr double INCHES_PER_PT = 1 / 72.0;
    constexpr double INCHES_PER_PICA = 1 / 12.0;

    constexpr double display_dpi = 96;
    // RSvgLength
    gboolean out_has_width;
    RsvgLength out_width;
    gboolean out_has_height;
    RsvgLength out_height;
    gboolean out_has_viewbox;
    RsvgRectangle out_viewbox;
    LvtkSize result{24, 24};
    rsvg_handle_get_intrinsic_dimensions(
        this->handle,
        &out_has_width,
        &out_width,
        &out_has_height,
        &out_height,
        &out_has_viewbox,
        &out_viewbox);
    if (out_has_width)
    {
        switch (out_width.unit)
        {
        case RsvgUnit::RSVG_UNIT_PX:
            result.Width(out_width.length);
            break;
        case RsvgUnit::RSVG_UNIT_IN:
            result.Width(out_width.length * display_dpi);
            break;
        case RsvgUnit::RSVG_UNIT_CM:
            result.Width(out_width.length * (display_dpi * INCHES_PER_CM));
            break;
        case RsvgUnit::RSVG_UNIT_MM:
            result.Width(out_width.length * (display_dpi * INCHES_PER_MM));
            break;
        case RsvgUnit::RSVG_UNIT_PT:
            result.Width(out_width.length * (display_dpi * INCHES_PER_PT));
            break;
        case RsvgUnit::RSVG_UNIT_PC:
            result.Width(out_width.length * (display_dpi * INCHES_PER_PICA));
            break;
        case RsvgUnit::RSVG_UNIT_PERCENT:
        case RsvgUnit::RSVG_UNIT_EM:
        case RsvgUnit::RSVG_UNIT_EX:
            throw std::runtime_error("Not implemented.");
        default:
            break;
        }
    }
    if (out_has_height)
    {
        switch (out_height.unit)
        {
        case RsvgUnit::RSVG_UNIT_PX:
            result.Height(out_height.length);
            break;
        case RsvgUnit::RSVG_UNIT_IN:
            result.Height(out_height.length * display_dpi);
            break;
        case RsvgUnit::RSVG_UNIT_CM:
            result.Height(out_height.length * (display_dpi * INCHES_PER_CM));
            break;
        case RsvgUnit::RSVG_UNIT_MM:
            result.Height(out_height.length * (display_dpi * INCHES_PER_MM));
            break;
        case RsvgUnit::RSVG_UNIT_PT:
            result.Height(out_height.length * (display_dpi * INCHES_PER_PT));
            break;
        case RsvgUnit::RSVG_UNIT_PC:
            result.Height(out_height.length * (display_dpi * INCHES_PER_PICA));
            break;
        case RsvgUnit::RSVG_UNIT_PERCENT:
        case RsvgUnit::RSVG_UNIT_EM:
        case RsvgUnit::RSVG_UNIT_EX:
            throw std::runtime_error("Not implemented.");
        default:
            break;
        }
    }
    return result;
}

void LvtkSvg::render(LvtkDrawingContext &context, const LvtkRectangle &viewport)
{
    GError *error = nullptr;
    RsvgRectangle rc;
    rc.x = viewport.Left();
    rc.y = viewport.Top();
    rc.width = viewport.Width();
    rc.height = viewport.Height();
    if (!rsvg_handle_render_document(this->handle, context.get(), &rc, &error))
    {
        throw std::runtime_error(SS(error->message << "(" << error->code << ")"));
    }
}
