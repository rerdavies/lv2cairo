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

#include "lv2c/Lv2cDrawingContext.hpp"
#include "lv2c/Lv2cLog.hpp"
#include "cairo/cairo.h"
#include <numbers>
#include "ss.hpp"

using namespace lvtk;

Lv2cPattern Lv2cPattern::radial_gradient(double cx, double cy, double radius, const std::vector<Lv2cColorStop> &colorStops)
{
    Lv2cPattern pattern = cairo_pattern_create_radial(cx, cy, 0, cx, cy, radius);
    for (const Lv2cColorStop &colorStop : colorStops)
    {
        const Lv2cColor &color = colorStop.getColor();
        cairo_pattern_add_color_stop_rgba(pattern.get(), colorStop.getOffset(), color.R(), color.G(), color.B(), color.A());
    }
    return pattern;
}
Lv2cPattern Lv2cPattern::radial_gradient(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1, const std::vector<Lv2cColorStop> &colorStops)
{
    Lv2cPattern pattern = cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1);
    for (const Lv2cColorStop &colorStop : colorStops)
    {
        const Lv2cColor &color = colorStop.getColor();
        cairo_pattern_add_color_stop_rgba(pattern.get(), colorStop.getOffset(), color.R(), color.G(), color.B(), color.A());
    }
    return pattern;
}

Lv2cPattern Lv2cPattern::linear_gradient(double cx0, double cy0, double cx1, double cy1, const std::vector<Lv2cColorStop> &colorStops)
{
    Lv2cPattern pattern = cairo_pattern_create_linear(cx0, cy0, cx1, cy1);
    for (const Lv2cColorStop &colorStop : colorStops)
    {
        const Lv2cColor &color = colorStop.getColor();
        cairo_pattern_add_color_stop_rgba(pattern.get(), colorStop.getOffset(), color.R(), color.G(), color.B(), color.A());
    }
    return pattern;
}

Lv2cPoint Lv2cDrawingContext::round_to_device(Lv2cPoint point)
{
    Lv2cPoint ptDevice = this->user_to_device(point);
    ptDevice.x = std::floor(ptDevice.x);
    ptDevice.y = std::floor(ptDevice.y);
    return this->device_to_user(ptDevice);
}

Lv2cPoint Lv2cDrawingContext::to_device_ceiling(Lv2cPoint point)
{
    Lv2cPoint ptDevice = this->user_to_device(point);
    ptDevice.x = std::ceil(ptDevice.x);
    ptDevice.y = std::ceil(ptDevice.y);
    return this->device_to_user(ptDevice);
}
Lv2cPoint Lv2cDrawingContext::to_device_floor(Lv2cPoint point)
{
    Lv2cPoint ptDevice = this->user_to_device(point);
    ptDevice.x = std::floor(ptDevice.x);
    ptDevice.y = std::floor(ptDevice.y);
    return this->device_to_user(ptDevice);
}

Lv2cRectangle Lv2cDrawingContext::round_to_device(const Lv2cRectangle &rectangle)
{
    double left = rectangle.Left();
    double top = rectangle.Top();
    double right = rectangle.Right();
    double bottom = rectangle.Bottom();
    user_to_device(&left, &top);
    user_to_device(&right, &bottom);
    left = std::floor(left);
    top = std::floor(top);
    right = std::ceil(right);
    bottom = std::ceil(bottom);

    device_to_user(&left, &top);
    device_to_user(&right, &bottom);

    return Lv2cRectangle(left, top, right - left, bottom - top);
}

Lv2cPoint Lv2cDrawingContext::device_to_user(Lv2cPoint pt)
{
    device_to_user(&pt.x, &pt.y);
    return pt;
}

Lv2cPoint Lv2cDrawingContext::user_to_device(Lv2cPoint pt)
{
    user_to_device(&pt.x, &pt.y);
    return pt;
}

Lv2cRectangle Lv2cDrawingContext::user_to_device(const Lv2cRectangle &rectangle)
{
    Lv2cPoint pt0{rectangle.Left(), rectangle.Top()};
    Lv2cPoint pt1{rectangle.Right(), rectangle.Bottom()};

    pt0 = user_to_device(pt0);
    pt1 = user_to_device(pt1);

    return Lv2cRectangle(pt0.x, pt0.y, pt1.x - pt0.x, pt1.y - pt0.y);
}
Lv2cRectangle Lv2cDrawingContext::device_to_user(const Lv2cRectangle &rectangle)
{
    Lv2cPoint pt0{rectangle.Left(), rectangle.Top()};
    Lv2cPoint pt1{rectangle.Right(), rectangle.Bottom()};

    pt0 = device_to_user(pt0);
    pt1 = device_to_user(pt1);

    return Lv2cRectangle(pt0.x, pt0.y, pt1.x - pt0.x, pt1.y - pt0.y);
}

Lv2cSurface::Lv2cSurface() : surface(nullptr) {}
Lv2cSurface::Lv2cSurface(const Lv2cSurface &other)
{
    this->surface = const_cast<cairo_surface_t *>(other.surface);
    cairo_surface_reference(this->surface);
}
Lv2cSurface::Lv2cSurface(Lv2cSurface &&other)
{
    this->surface = nullptr;
    std::swap(surface, other.surface);
}
Lv2cSurface &Lv2cSurface::operator=(const Lv2cSurface &other)
{
    release();
    this->surface = const_cast<cairo_surface_t *>(other.surface);
    cairo_surface_reference(this->surface);
    return *this;
}
Lv2cSurface &Lv2cSurface::operator=(Lv2cSurface &&other)
{
    std::swap(surface, other.surface);
    return *this;
}
Lv2cSurface::Lv2cSurface(cairo_surface_t *surface) : surface(surface) {}
void Lv2cSurface::release()
{
    if (surface)
    {
        cairo_surface_destroy(surface);
        surface = nullptr;
    }
}

Lv2cSurface::~Lv2cSurface()
{
    release();
}

Lv2cSize Lv2cSurface::size() const
{
    auto width = cairo_image_surface_get_width(surface);
    auto height = cairo_image_surface_get_height(surface);
    return Lv2cSize(width, height);
}

Lv2cSurface Lv2cSurface::create_from_png(const char *filename)
{
    cairo_surface_t *surface = cairo_image_surface_create_from_png(filename);
    return Lv2cSurface(surface);
}
Lv2cSurface Lv2cSurface::create_from_png(const std::string &filename)
{
    cairo_surface_t *surface = cairo_image_surface_create_from_png(filename.c_str());
    return Lv2cSurface(surface);
}

#define CAIRO_STATUS_CASE(x) \
    case cairo_status_t::x:  \
        return #x;

const std::string lvtk::Lv2cStatusMessage(cairo_status_t status)
{
    switch (status)
    {
        CAIRO_STATUS_CASE(CAIRO_STATUS_SUCCESS)

        CAIRO_STATUS_CASE(CAIRO_STATUS_NO_MEMORY)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_RESTORE)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_POP_GROUP)
        CAIRO_STATUS_CASE(CAIRO_STATUS_NO_CURRENT_POINT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_MATRIX)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_STATUS)
        CAIRO_STATUS_CASE(CAIRO_STATUS_NULL_POINTER)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_STRING)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_PATH_DATA)
        CAIRO_STATUS_CASE(CAIRO_STATUS_READ_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_WRITE_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_SURFACE_FINISHED)
        CAIRO_STATUS_CASE(CAIRO_STATUS_SURFACE_TYPE_MISMATCH)
        CAIRO_STATUS_CASE(CAIRO_STATUS_PATTERN_TYPE_MISMATCH)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_CONTENT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_FORMAT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_VISUAL)
        CAIRO_STATUS_CASE(CAIRO_STATUS_FILE_NOT_FOUND)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_DASH)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_DSC_COMMENT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_INDEX)
        CAIRO_STATUS_CASE(CAIRO_STATUS_CLIP_NOT_REPRESENTABLE)
        CAIRO_STATUS_CASE(CAIRO_STATUS_TEMP_FILE_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_STRIDE)
        CAIRO_STATUS_CASE(CAIRO_STATUS_FONT_TYPE_MISMATCH)
        CAIRO_STATUS_CASE(CAIRO_STATUS_USER_FONT_IMMUTABLE)
        CAIRO_STATUS_CASE(CAIRO_STATUS_USER_FONT_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_NEGATIVE_COUNT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_CLUSTERS)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_SLANT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_WEIGHT)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_SIZE)
        CAIRO_STATUS_CASE(CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED)
        CAIRO_STATUS_CASE(CAIRO_STATUS_DEVICE_TYPE_MISMATCH)
        CAIRO_STATUS_CASE(CAIRO_STATUS_DEVICE_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_INVALID_MESH_CONSTRUCTION)
        CAIRO_STATUS_CASE(CAIRO_STATUS_DEVICE_FINISHED)
        CAIRO_STATUS_CASE(CAIRO_STATUS_JBIG2_GLOBAL_MISSING)
        CAIRO_STATUS_CASE(CAIRO_STATUS_PNG_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_FREETYPE_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_WIN32_GDI_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_TAG_ERROR)
        CAIRO_STATUS_CASE(CAIRO_STATUS_LAST_STATUS)
    default:
        return "Unknown error.";
    }
}

cairo_pattern_type_t Lv2cPattern::get_type() const
{
    return cairo_pattern_get_type(pattern);
}

Lv2cColor Lv2cPattern::get_color() const
{
    if (cairo_pattern_get_type(pattern) == cairo_pattern_type_t::CAIRO_PATTERN_TYPE_SOLID)
    {
        double r = 0, g = 0, b = 0, a = 1;
        cairo_pattern_get_rgba(pattern, &r, &g, &b, &a);
        return Lv2cColor(r, g, b, a);
    }
    LogError("Called Lv2cPattern::get_color on a pattern that isn't a solid color");
    return Lv2cColor(1, 0.5, 0.5, 1);
}

static double degreesToRadians(double angle)
{
    return angle * (std::numbers::pi / 180.0);
}
static void FitRadii(double available, double &v1, double &v2)
{
    if (v1 + v2 > available)
    {
        if (available <= 0.001)
        {
            v1 = 0;
            v2 = 0;
        }
        else
        {
            double scale = (available) / (v1 + v2);
            v1 *= scale;
            v2 *= scale;
        }
    }
}

void Lv2cDrawingContext::round_corner_rectangle(const Lv2cRectangle &rectangle, const Lv2cRoundCorners &corners_)
{
    Lv2cRoundCorners corners = corners_;
    // reduce radii if neccessary.
    FitRadii(rectangle.Width(), corners.topLeft, corners.topRight);
    FitRadii(rectangle.Width(), corners.bottomLeft, corners.bottomRight);
    FitRadii(rectangle.Height(), corners.topLeft, corners.topRight);
    FitRadii(rectangle.Height(), corners.topRight, corners.bottomRight);

    this->move_to(rectangle.Left() + corners.topLeft, rectangle.Top());
    this->arc(
        rectangle.Right() - corners.topRight,
        rectangle.Top() + corners.topRight,
        corners.topRight,
        degreesToRadians(-90),
        degreesToRadians(0));

    this->arc(
        rectangle.Right() - corners.bottomRight,
        rectangle.Bottom() - corners.bottomRight,
        corners.bottomRight,
        degreesToRadians(0),
        degreesToRadians(90));

    this->arc(
        rectangle.Left() + corners.bottomLeft,
        rectangle.Bottom() - corners.bottomLeft,
        corners.bottomLeft,
        degreesToRadians(90),
        degreesToRadians(180));

    this->arc(
        rectangle.Left() + corners.topLeft,
        rectangle.Top() + corners.topLeft,
        corners.topLeft,
        degreesToRadians(180),
        degreesToRadians(270));
    this->close_path();
}

void Lv2cSurface::throw_status_error()
{
    if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
    {
        throw std::runtime_error(SS("Lv2c: " << Lv2cStatusMessage(status())));
    }
}

void Lv2cDrawingContext::throw_status_error()
{
    if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
    {
        throw std::runtime_error(SS("Lv2c: " << Lv2cStatusMessage(status())));
    }
}

void Lv2cDrawingContext::log_status_error()
{
    if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
    {
        LogError(SS("Lv2c: " << Lv2cStatusMessage(status())));
    }
}

Lv2cImageSurface::Lv2cImageSurface(
    cairo_format_t format,
    int width,
    int height)
{
    surface = cairo_image_surface_create(format, width, height);
    check_status();
}



