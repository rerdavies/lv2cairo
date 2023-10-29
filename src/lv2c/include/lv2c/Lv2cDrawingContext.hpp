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
#include "cairo/cairo.h"
#include <vector>

namespace lvtk
{

    class Lv2cColorStop
    {
    public:
        Lv2cColorStop(double offset, const Lv2cColor &color) : offset(offset), color(color) {}

        double getOffset() const { return offset; }
        const Lv2cColor &getColor() const { return color; }

    private:
        double offset;
        Lv2cColor color;
    };

    const std::string Lv2cStatusMessage(cairo_status_t status);

    class Lv2cSurface
    {
    public:
        Lv2cSurface();
        Lv2cSurface(const Lv2cSurface &other);
        Lv2cSurface(Lv2cSurface &&other);
        Lv2cSurface &operator=(const Lv2cSurface &other);
        Lv2cSurface &operator=(Lv2cSurface &&other);
        Lv2cSurface(cairo_surface_t *surface);
        ~Lv2cSurface();

        cairo_status_t status() { return cairo_surface_status(surface); }

        // Throw an error if the status() is not successful.
        void check_status();

        cairo_surface_t *get() { return surface; }
        void release();
        operator bool() const { return surface != nullptr; }

        Lv2cSize size() const;
        static Lv2cSurface create_from_png(const char *filename);
        static Lv2cSurface create_from_png(const std::string &filename);

        cairo_surface_type_t
        get_type() { return cairo_surface_get_type(surface); }

        cairo_content_t
        get_content()
        {
            return cairo_surface_get_content(surface);
        }

#if CAIRO_HAS_PNG_FUNCTIONS

        cairo_status_t
        write_to_png(const char *filename)
        {
            return cairo_surface_write_to_png(surface, filename);
        }
#endif
        void
        flush()
        {
            cairo_surface_flush(surface);
        }
        void
        mark_dirty() {
            cairo_surface_mark_dirty(surface);
        }
        void 
        mark_dirty_rectangle(
            int              x,
            int              y,
            int              width,
            int              height)
        {
            cairo_surface_mark_dirty_rectangle(
                surface,
                x,y,
                width,height);
        }



        void set_device_scale(double x_scale, double y_scale)
        {
            cairo_surface_set_device_scale(surface,
                                           x_scale,
                                           y_scale);
        }

        void
        get_device_scale(double *x_scale, double *y_scale)
        {
            cairo_surface_get_device_scale(surface,
                                           x_scale,
                                           y_scale);
        }

        void set_device_offset(double x_offset, double y_offset)
        {
            cairo_surface_set_device_offset(surface, x_offset, y_offset);
        }

        void
        surface_get_device_offset(double *x_offset, double *y_offset)
        {
            cairo_surface_get_device_offset(surface,
                                            x_offset,
                                            y_offset);
        }

    protected:
        cairo_surface_t *surface;
    private:
        void throw_status_error();
    };

    class Lv2cImageSurface : public Lv2cSurface
    {
    public:
        Lv2cImageSurface(
            cairo_format_t format,
            int width,
            int height);

        Lv2cImageSurface(const Lv2cSurface &other) : Lv2cSurface(other) {}
        Lv2cImageSurface(Lv2cSurface &&other) : Lv2cSurface(std::move(other)) {}
        Lv2cImageSurface(cairo_surface_t *surface) : Lv2cSurface(surface) {}

        static int
        stride_for_width(
            cairo_format_t format,
            int width)
        {
            return cairo_format_stride_for_width(format, width);
        }
        Lv2cImageSurface(unsigned char *data,
                          cairo_format_t format,
                          int width,
                          int height,
                          int stride)
        {
            surface = cairo_image_surface_create_for_data(
                data,
                format,
                width,
                height,
                stride);
        }

        unsigned char *get_data()
        {
            return cairo_image_surface_get_data(surface);
        }

        cairo_format_t
        get_format()
        {
            return cairo_image_surface_get_format(surface);
        }

        int get_width()
        {
            return cairo_image_surface_get_width(surface);
        }

        int get_height()
        {
            return cairo_image_surface_get_height(surface);
        }

        int get_stride()
        {
            return cairo_image_surface_get_stride(surface);
        }
    };

    class Lv2cMatrix
    {
    public:
        Lv2cMatrix() { cairo_matrix_init_identity(&matrix); }

        void init_identity() { cairo_matrix_init_identity(&matrix); }
        void init_translate(double tx, double ty) { cairo_matrix_init_translate(&matrix, tx, ty); }
        void init_scale(double sx, double sy) { cairo_matrix_init_scale(&matrix, sx, sy); }
        void init_rotate(double radians) { cairo_matrix_init_rotate(&matrix, radians); }

        void translate(double tx, double ty) { cairo_matrix_translate(&matrix, tx, ty); }
        void scale(double sx, double sy) { cairo_matrix_scale(&matrix, sx, sy); }

        void rotate(double radians) { cairo_matrix_rotate(&matrix, radians); }
        Lv2cPoint transform_distance(const Lv2cPoint &point);
        Lv2cPoint transform_point(const Lv2cPoint &point);

        operator cairo_matrix_t *() { return &matrix; }
        operator const cairo_matrix_t *() const { return &matrix; }

        const cairo_matrix_t *get() const { return &matrix; }
        cairo_matrix_t *get() { return &matrix; }

    private:
        cairo_matrix_t matrix;
    };
    inline Lv2cMatrix operator*(const Lv2cMatrix &a, const Lv2cMatrix &b)
    {
        Lv2cMatrix result;
        cairo_matrix_multiply(result.get(), a.get(), b.get());
        return result;
    }

    class Lv2cPattern
    {
    public:
        Lv2cPattern() : pattern(nullptr) {}
        Lv2cPattern(cairo_pattern_t *pattern) : pattern(pattern) {}
        Lv2cPattern(Lv2cPattern &&other) : pattern(nullptr) { std::swap(this->pattern, other.pattern); }
        Lv2cPattern(const Lv2cPattern &other) : pattern(other.pattern == nullptr ? nullptr : cairo_pattern_reference(const_cast<cairo_pattern_t *>(other.pattern))) {}
        Lv2cPattern(const Lv2cColor &color) : pattern(cairo_pattern_create_rgba(color.R(), color.G(), color.B(), color.A())) {}
        Lv2cPattern(cairo_surface_t *surface) : pattern(cairo_pattern_create_for_surface(surface)) {}
        Lv2cPattern(Lv2cSurface &surface) : pattern(cairo_pattern_create_for_surface(surface.get())) {}
        ~Lv2cPattern() { release(); }

        Lv2cPattern &operator=(const Lv2cPattern &other)
        {
            release();
            set(other);
            return *this;
        }
        Lv2cPattern &operator=(cairo_pattern_t *pattern)
        {
            release();
            set(pattern);
            return *this;
        }
        Lv2cPattern &operator=(Lv2cPattern &&other)
        {
            std::swap(this->pattern, other.pattern);
            return *this;
        }

        cairo_pattern_t *get() { return pattern; }
        cairo_pattern_t *get() const { return const_cast<cairo_pattern_t *>(pattern); }
        operator bool() const { return pattern != nullptr; }

        static Lv2cPattern linear_gradient(double cx0, double cy0, double cx1, double cy1, const std::vector<Lv2cColorStop> &colorStops);
        static Lv2cPattern radial_gradient(double cx, double cy, double radius, const std::vector<Lv2cColorStop> &colorStops);
        static Lv2cPattern radial_gradient(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1, const std::vector<Lv2cColorStop> &colorStops);

        void set_extended(cairo_extend_t extend) { cairo_pattern_set_extend(pattern, extend); }
        cairo_extend_t get_extended() { return cairo_pattern_get_extend(pattern); }

        void set_filter(cairo_filter_t filter) { cairo_pattern_set_filter(pattern, filter); }
        cairo_filter_t get_filter() { return cairo_pattern_get_filter(pattern); }

        void set_matrix(Lv2cMatrix &matrix) { cairo_pattern_set_matrix(pattern, matrix.get()); }
        Lv2cMatrix get_matrix()
        {
            Lv2cMatrix matrix;
            cairo_pattern_get_matrix(pattern, matrix.get());
            return matrix;
        }
        cairo_pattern_type_t get_type() const;
        void get_matrix(cairo_matrix_t *matrix) { cairo_pattern_get_matrix(pattern, matrix); }

        Lv2cColor get_color() const;

        bool isEmpty() const { return pattern == nullptr; }
        int reference_count() { return pattern == nullptr ? 0 : cairo_pattern_get_reference_count(pattern); }

    private:
        void release()
        {
            if (pattern)
                cairo_pattern_destroy(pattern);
            pattern = nullptr;
        }
        void set(cairo_pattern_t *pattern) { pattern = pattern == nullptr ? nullptr : cairo_pattern_reference(pattern); }
        void set(const Lv2cPattern &other) { pattern = other.pattern == nullptr ? nullptr : cairo_pattern_reference(const_cast<cairo_pattern_t *>(other.pattern)); }

    private:
        cairo_pattern_t *pattern = nullptr;
    };

    class Lv2cRectangleList
    {
    public:
        Lv2cRectangleList(cairo_rectangle_list_t *list) : list(list) {}
        ~Lv2cRectangleList()
        {
            if (list)
                cairo_rectangle_list_destroy(list);
        }
        cairo_rectangle_list_t *get() { return list; }

    private:
        cairo_rectangle_list_t *list = nullptr;
    };

    class Lv2cScaledFont
    {
    public:
        Lv2cScaledFont(cairo_scaled_font_t *font) : font(font) {}
        ~Lv2cScaledFont()
        {
            if (font)
                cairo_scaled_font_destroy(font);
        }
        Lv2cScaledFont(Lv2cScaledFont &other) : font(cairo_scaled_font_reference(other.get()))
        {
        }
        cairo_scaled_font_t *get() { return font; }

    private:
        cairo_scaled_font_t *font;
    };

    class Lv2cDrawingContext
    {
    public:
        Lv2cDrawingContext(cairo_t *context)
        {
            this->context = context;
        }
        Lv2cDrawingContext(cairo_surface_t *target)
        {
            context = cairo_create(target);
        }
        Lv2cDrawingContext(Lv2cSurface &target)
        {
            context = cairo_create(target.get());
        }



        Lv2cDrawingContext(Lv2cDrawingContext &other) { context = cairo_reference(other.context); }
        ~Lv2cDrawingContext() { cairo_destroy(context); }

        Lv2cDrawingContext &operator=(Lv2cDrawingContext &other)
        {
            if (context)
                cairo_destroy(context);
            context = cairo_reference(other.context);
            return *this;
        }
        cairo_status_t status() { return cairo_status(context); }
        // Throw an exception if the dc status is not sucess.
        void check_status();
        void log_status();

        int get_reference_count() { return cairo_get_reference_count(context); }

        void *get_user_data(const cairo_user_data_key_t *key) { return cairo_get_user_data(context, key); }
        cairo_status_t set_user_data(
            const cairo_user_data_key_t *key,
            void *user_data,
            cairo_destroy_func_t destroy)
        {
            return cairo_set_user_data(context, key, user_data, destroy);
        }
        int save()
        {
            cairo_save(context);
            return ++saveCount;
        }
        int restore()
        {
            cairo_restore(context);
            int value = saveCount--;
            if (saveCount < 0)
            {
                throw std::runtime_error("Unbalanced restore.");
            }
            return value;
        }

        void push_group() { cairo_push_group(context); }
        void push_group_with_content(cairo_content_t content) { cairo_push_group_with_content(context, content); }
        Lv2cPattern pop_group() { return Lv2cPattern(cairo_pop_group(context)); }
        void pop_group_to_source() { cairo_pop_group_to_source(context); }

        void set_operator(cairo_operator_t op) { cairo_set_operator(context, op); }
        cairo_operator_t get_operator() { return cairo_get_operator(context); }

        void set_source(const Lv2cColor &color) { cairo_set_source_rgba(context, color.R(), color.G(), color.B(), color.A()); }
        void set_source(float r, float g, float b) { cairo_set_source_rgb(context, r, g, b); }
        void set_source(float r, float g, float b, float a) { cairo_set_source_rgba(context, r, g, b, a); }
        void set_source(const Lv2cPattern &pattern) { cairo_set_source(context, pattern.get()); }

        void set_source(cairo_surface_t *surface, double x, double y)
        {
            cairo_set_source_surface(context, surface, x, y);
        }
        void set_source(Lv2cSurface &surface, double x, double y)
        {
            cairo_set_source_surface(context, surface.get(), x, y);
            cairo_get_source(context);
        }
        Lv2cPattern get_source()
        {
            cairo_pattern_t *source = cairo_get_source(context);
            cairo_pattern_reference(source); // returned value is un-addref'ed!.
            return Lv2cPattern(source);
        }

        void set_toloerance(double tolerance) { cairo_set_tolerance(context, tolerance); }

        void set_antialias(cairo_antialias_t antialias) { cairo_set_antialias(context, antialias); }
        cairo_antialias_t get_antialias() { return cairo_get_antialias(context); }

        void set_fill_rule(cairo_fill_rule_t fill_rule) { cairo_set_fill_rule(context, fill_rule); }
        cairo_fill_rule_t get_fill_rule() { return cairo_get_fill_rule(context); }

        void set_line_width(double width) { cairo_set_line_width(context, width); }

        void set_line_cap(cairo_line_cap_t line_cap) { cairo_set_line_cap(context, line_cap); }
        void set_line_join(cairo_line_join_t line_join) { cairo_set_line_join(context, line_join); }

        void set_dash(const std::vector<double> &dashes, double offset)
        {
            cairo_set_dash(context, dashes.data(), (int)dashes.size(), offset);
        }
        void set_miter_limit(double limit) { cairo_set_miter_limit(context, limit); }

        void translate(double tx, double ty) { cairo_translate(context, tx, ty); }
        void scale(double sx, double sy) { cairo_scale(context, sx, sy); }
        void rotate(double angle) { cairo_rotate(context, angle); }
        void transform(const cairo_matrix_t *matrix) { cairo_transform(context, matrix); }
        void set_matrix(const cairo_matrix_t *matrix) { cairo_set_matrix(context, matrix); }
        void identity_matrix() { cairo_identity_matrix(context); }

        Lv2cRectangle round_to_device(const Lv2cRectangle &rectangle);
        Lv2cPoint round_to_device(Lv2cPoint pt);
        Lv2cPoint to_device_ceiling(Lv2cPoint pt);
        Lv2cPoint to_device_floor(Lv2cPoint pt);

        void user_to_device(double *x, double *y) { cairo_user_to_device(context, x, y); }
        void device_to_user(double *x, double *y) { cairo_device_to_user(context, x, y); }
        Lv2cRectangle user_to_device(const Lv2cRectangle &rectangle);
        Lv2cRectangle device_to_user(const Lv2cRectangle &rectangle);

        void device_to_user_distance(double *x, double *y) { cairo_device_to_user_distance(context, x, y); }
        Lv2cPoint device_to_user_distance(Lv2cPoint pt)
        {
            cairo_device_to_user_distance(context, &pt.x, &pt.y);
            return pt;
        }
        void user_to_device_distance(double *x, double *y) { cairo_user_to_device_distance(context, x, y); }
        Lv2cPoint user_to_device_distance(Lv2cPoint pt)
        {
            cairo_user_to_device_distance(context, &pt.x, &pt.y);
            return pt;
        }

        Lv2cPoint device_to_user(Lv2cPoint pt);
        Lv2cPoint user_to_device(Lv2cPoint pt);

        void new_path() { cairo_new_path(context); }
        void move_to(double x, double y) { cairo_move_to(context, x, y); }
        void new_sub_path() { cairo_new_sub_path(context); }
        void line_to(double x, double y) { cairo_line_to(context, x, y); }
        void curve_to(
            double x1, double y1,
            double x2, double y2,
            double x3, double y3)
        {
            cairo_curve_to(context, x1, y1, x2, y2, x3, y3);
        }
        void
        arc(
            double xc, double yc,
            double radius,
            double angle1, double angle2)
        {
            cairo_arc(context, xc, yc, radius, angle1, angle2);
        }

        void
        arc_negative(
            double xc, double yc,
            double radius,
            double angle1, double angle2)
        {
            cairo_arc_negative(context, xc, yc, radius, angle1, angle2);
        }
        void rel_move_to(double dx, double dy) { cairo_rel_move_to(context, dx, dy); }

        void rel_line_to(double dx, double dy) { cairo_rel_line_to(context, dx, dy); }

        void rel_curve_to(
            double dx1, double dy1,
            double dx2, double dy2,
            double dx3, double dy3)
        {
            cairo_rel_curve_to(context, dx1, dy1, dx2, dy2, dx3, dy3);
        }
        void rectangle(const Lv2cRectangle &rectangle)
        {
            cairo_rectangle(context, rectangle.X(), rectangle.Y(), rectangle.Width(), rectangle.Height());
        }
        void round_corner_rectangle(const Lv2cRectangle &rectangle, const Lv2cRoundCorners &corners);
        void rectangle(
            double x, double y,
            double width, double height)
        {
            cairo_rectangle(context, x, y, width, height);
        }
        void close_path() { cairo_close_path(context); }

        void paint() { cairo_paint(context); }
        void paint_with_alpha(double alpha) { cairo_paint_with_alpha(context, alpha); }

        void mask(Lv2cPattern &pattern) { cairo_mask(context, pattern.get()); }
        void mask_surface(cairo_surface_t *surface, double surface_x, double surface_y)
        {
            cairo_mask_surface(context, surface, surface_x, surface_y);
        }
        void mask_surface(Lv2cSurface &surface, double surface_x, double surface_y)
        {
            cairo_mask_surface(context, surface.get(), surface_x, surface_y);
        }
        void stroke() { cairo_stroke(context); }
        void stroke_preserve() { cairo_stroke_preserve(context); }
        void fill() { cairo_fill(context); }
        void fill_preserve() { cairo_fill_preserve(context); }
        void copy_page() { cairo_copy_page(context); }
        void show_page() { cairo_show_page(context); }
        bool in_stroke(double x, double y) { return cairo_in_stroke(context, x, y); }
        bool in_fill(double x, double y) { return cairo_in_fill(context, x, y); }
        bool in_clip(double x, double y) { return cairo_in_clip(context, x, y); }
        void stroke_extents(double *x1, double *y1, double *x2, double *y2)
        {
            cairo_stroke_extents(context, x1, y1, x2, y2);
        }
        void fill_extents(double *x1, double *y1, double *x2, double *y2)
        {
            cairo_fill_extents(context, x1, y1, x2, y2);
        }
        void reset_clip() { cairo_reset_clip(context); }
        void clip() { cairo_clip(context); }
        void clip_preserve() { cairo_clip_preserve(context); }
        void clip_extents(
            double *x1, double *y1,
            double *x2, double *y2)
        {
            cairo_clip_extents(context, x1, y1, x2, y2);
        }

    public:
        cairo_t *get() { return context; }

    private:
        int saveCount = 0;
        void throw_status_error();
        void log_status_error();
        cairo_t *context;
    };

    ///////////////////////////////////////////////////////

    inline Lv2cPoint Lv2cMatrix::transform_distance(const Lv2cPoint &point)
    {
        Lv2cPoint result = point;
        cairo_matrix_transform_distance(&matrix, &result.x, &result.y);
        return result;
    }
    inline Lv2cPoint Lv2cMatrix::transform_point(const Lv2cPoint &point)
    {
        Lv2cPoint result = point;
        cairo_matrix_transform_point(&matrix, &result.x, &result.y);
        return result;
    }

    inline void Lv2cDrawingContext::check_status()
    {
        if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            throw_status_error();
        }
    }

    inline void Lv2cSurface::check_status()
    {
        if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            throw_status_error();
        }
    }

    inline void Lv2cDrawingContext::log_status()
    {
        if (status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            log_status_error();
        }
    }

} // namespace