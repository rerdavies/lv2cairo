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

#include "lv2c/Lv2cDropShadowElement.hpp"
#include <cmath>
#include "cleanup.hpp"
#include <memory.h>
#include <numbers>
#include "lv2c/Lv2cWindow.hpp"

using namespace lv2c;

static double ShadowFn(int x_, int y_, int radius)
{
    double x = x_;
    double y = y_;
    double d = std::sqrt(x * x + y * y) / radius;
    if (d < radius-0.5) return 1;
    if (d > radius+0.5) return 0;
    return d-(radius-0.5);
    // if (d > 1)
    //     return 0;
    // // return 0.5 + 0.5 * std::cos(d * std::numbers::pi);
    // return 1 - d;
}
void Lv2cDropShadowElement::BlurDropShadow(Lv2cDrawingContext &dc, cairo_surface_t *surface, double *pXOffset, double *pYOffset)
{
    double radius = Radius() * Window()->WindowScale();
    if (radius < 0.5)
        radius = 0.5;
    int64_t iRadius = (int)std::ceil(radius);
    if (iRadius < 1)
        iRadius = 1;

    // using SIGNED coordinates throughout because filters (may) use negative indices
    int64_t width = cairo_image_surface_get_width(surface);
    int64_t height = cairo_image_surface_get_height(surface);
    int64_t stride = cairo_image_surface_get_stride(surface);

    double xOffset = XOffset() * Window()->WindowScale();
    double yOffset = YOffset() * Window()->WindowScale();

    double xOffsetFrac = xOffset - std::round(xOffset);
    double yOffsetFrac = yOffset - std::round(yOffset);
    xOffset -= xOffsetFrac;
    yOffset -= yOffsetFrac;

    *pXOffset = xOffset / Window()->WindowScale();
    *pYOffset = yOffset / Window()->WindowScale();

    std::vector<uint8_t> workingBuffer;

    int64_t workingBufferStride = stride;

    workingBuffer.resize(workingBufferStride * (height));

    uint8_t *surfaceBuffer = (uint8_t *)cairo_image_surface_get_data(surface);

    memcpy(&(workingBuffer[0]), surfaceBuffer, workingBufferStride * height);

    int64_t filterSize = iRadius * 2;
    std::vector<float> filter;
    filter.resize(filterSize * filterSize);

    double norm = 0;

    for (int64_t r = 0; r < filterSize; ++r)
    {
        for (int64_t c = 0; c < filterSize; ++c)
        {
            float y = ShadowFn(r - iRadius - xOffsetFrac, c - iRadius - yOffsetFrac, radius);
            filter[r * filterSize + c] = y;
            norm += y;
        }
    }
    norm = 1 / norm;
    for (size_t i = 0; i < filter.size(); ++i)
    {
        filter[i] *= norm;
    }

    // translate to [radius,radius].

    for (int64_t row = 0; row < height; ++row)
    {

        for (int64_t column = 0; column < width; ++column)
        {
            float sum = 0;
            for (int64_t filterY = 0; filterY < filterSize; ++filterY)
            {
                int64_t sourceRow = row - iRadius + filterY;
                if (sourceRow >= 0 && sourceRow < height)
                {
                    auto pFilterSource = &(filter[filterSize * filterY]);
                    uint8_t *pRowSource = &(workingBuffer[sourceRow * workingBufferStride]);
                    int64_t sourceX = -iRadius + column;
                    if (sourceX >= 0 && sourceX + filterSize < width)
                    {
                        pRowSource += sourceX;
                        // fast, probably vectorized version.
                        for (int64_t filterX = 0; filterX < filterSize; ++filterX)
                        {
                            float v = (*pRowSource++) * (*pFilterSource++);
                            sum += v;
                        }
                    }
                    else
                    {
                        // slow, guarded version.
                        for (int64_t filterX = 0; filterX < filterSize; ++filterX)
                        {
                            int64_t tx = sourceX + filterX;
                            if (tx >= 0 && tx < width)
                            {
                                float v = pRowSource[tx] * (*pFilterSource);
                                sum += v;
                            } else {

                            }
                            pFilterSource++;
                        }
                    }
                }
            }
            // write back to the buffer.
            uint64_t value = (uint64_t)(sum);
            size_t surfaceIx = row * stride + column;

            if (value > 255)
                value = 255;
            // int64_t surfaceValue = surfaceBuffer[surfaceIx];
            // value = (surfaceValue*value)/255;
            surfaceBuffer[surfaceIx] = (uint8_t)value;
        }
    }
}

void Lv2cDropShadowElement::BlurInsetDropShadow(Lv2cDrawingContext &dc, cairo_surface_t *surface)
{
    double windowScale = Window()->WindowScale();
    double radius = Radius() * windowScale;
    int64_t iRadius = (int)std::ceil(radius);
    if (radius < 0.5)
    {
        radius = 0.5;
    }
    if (iRadius < 1)
        iRadius = 1;

    // using SIGNED coordinates throughout because filters (may) use negative indices
    int64_t width = cairo_image_surface_get_width(surface);
    int64_t height = cairo_image_surface_get_height(surface);
    int64_t stride = cairo_image_surface_get_stride(surface);
    uint8_t *surfaceBuffer = cairo_image_surface_get_data(surface);

    double xOffset = XOffset() * windowScale;
    double yOffset = YOffset() * windowScale;

    double xOffsetFrac = xOffset - std::round(xOffset);
    double yOffsetFrac = yOffset - std::round(yOffset);
    xOffset -= xOffsetFrac;
    yOffset -= yOffsetFrac;

    int64_t ixOffset = (int64_t)std::round(xOffset);
    int64_t iyOffset = (int64_t)std::round(yOffset);

    std::vector<uint8_t> workingBuffer;

    int64_t workingBufferSpan = stride;

    workingBuffer.resize(workingBufferSpan * height);
    memcpy(&workingBuffer[0], surfaceBuffer, workingBufferSpan * height);

    int64_t filterSize = iRadius * 2;
    std::vector<float> filter;
    filter.resize(filterSize * filterSize);

    double norm = 0;
    for (int64_t r = 0; r < filterSize; ++r)
    {
        for (int64_t c = 0; c < filterSize; ++c)
        {
            float y = ShadowFn(r - iRadius - xOffsetFrac, c - iRadius - yOffsetFrac, radius);
            filter[r * filterSize + c] = y;
            norm += y;
        }
    }
    norm = 1 / norm;
    for (size_t i = 0; i < filter.size(); ++i)
    {
        filter[i] *= norm;
    }

    for (int64_t row = 0; row < height; ++row)
    {

        for (int64_t column = 0; column < width; ++column)
        {
            float sum = 0;
            for (int64_t filterY = 0; filterY < filterSize; ++filterY)
            {
                auto pFilterSource = &(filter[filterSize * filterY]);
                int64_t sourceRow = row + filterY - iyOffset - iRadius;
                if (sourceRow >= height || sourceRow < 0)
                {
                    for (int64_t filterX = 0; filterX < filterSize; ++filterX)
                    {
                        sum += 255 * pFilterSource[filterX];
                    }
                }
                else
                {
                    auto pFilterSource = &(filter[filterSize * filterY]);
                    uint8_t *pRowSource = &(workingBuffer[sourceRow * workingBufferSpan]);
                    int64_t sourceX = column - iRadius - ixOffset;
                    if (sourceX >= 0 && sourceX + filterSize < width)
                    {
                        pRowSource += sourceX;
                        // fast, probably vectorized version.
                        for (int64_t filterX = 0; filterX < filterSize; ++filterX)
                        {
                            float v = (255 - *pRowSource++) * (*pFilterSource++);
                            sum += v;
                        }
                    }
                    else
                    {
                        // slow, guarded version.
                        for (int64_t filterX = 0; filterX < filterSize; ++filterX)
                        {
                            if (sourceX < 0 || sourceX >= width)
                            {
                                float v = 255 * (*pFilterSource++);
                                sum += v;
                            }
                            else
                            {
                                float v = (255 - pRowSource[sourceX]) * (*pFilterSource++);
                                sum += v;
                            }
                            ++sourceX;
                        }
                    }
                }
            }
            // write back to the buffer.
            int64_t value = (int64_t)(sum);
            size_t surfaceIx = row * stride + column;


            // (alpha values are linear. Review this).
            // We'll reconcile with the original by drawing ATOP.
            // int64_t surfaceValue = surfaceBuffer[surfaceIx];
            // value -= (255 - surfaceValue);
            // if (value < 0)
            //     value = 0;
            // if (value > 255)
            //     value = 255;

            surfaceBuffer[surfaceIx] = (uint8_t)value;
        }
    }
}

bool Lv2cDropShadowElement::DrawFastDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{
    // optimized drop shadow when there's a solid background (with or without round corners).
    if (!IsSolidBackground())
    {
        return false;
    }

    // build a  9-patch that we will use to render the shadow.
    Lv2cRoundCorners roundCorners = this->Style().RoundCorners().PixelValue();
    double nineLeft = std::max(roundCorners.topLeft, roundCorners.bottomLeft) + Radius();
    double nineRight = std::max(roundCorners.topRight, roundCorners.bottomRight) + Radius();
    double nineTop = std::max(roundCorners.topLeft, roundCorners.topRight) + Radius();
    double nineBottom = std::max(roundCorners.bottomLeft, roundCorners.bottomRight) + Radius();

    // if there's any ambiguity about the ninepatch, fall back to non-fast.
    if (nineRight+nineLeft+ 4 > screenBorderBounds.Width())
    {
        return false;
    }
    if (nineTop+nineBottom+4 > screenBorderBounds.Height())
    {
        return false;
    }
    Lv2cRectangle deviceBorderRectangle = dc.user_to_device(screenBorderBounds);
    Lv2cRectangle deviceShadowRect = dc.user_to_device(screenBorderBounds.Inflate(Radius()));
    Lv2cRectangle deviceNineRect = deviceShadowRect.Ceiling();

    Lv2cPoint deviceNineP1 = dc.user_to_device(
                                    Lv2cPoint(
                                        screenBorderBounds.Left() + nineLeft,
                                        screenBorderBounds.Top() + nineTop))
                                  .ceil();
    Lv2cPoint deviceNineP2 = dc.user_to_device(
                                    Lv2cPoint(
                                        screenBorderBounds.Right() - nineRight,
                                        screenBorderBounds.Bottom() - nineBottom))
                                  .floor();
    double dx1 = deviceNineP1.x - deviceNineRect.Left();
    double dx3 = deviceNineRect.Right() - deviceNineP2.x;
    double nineXs[4] = {
        0,
        dx1,
        dx1 + 3,
        dx1 + 3 + dx3};
    double dy1 = deviceNineP1.y - deviceNineRect.Top();
    double dy3 = deviceNineRect.Right() - deviceNineP2.x;
    double nineYs[4] = {
        0,
        dy1,
        dy1 + 3,
        dy1 + 3 + dy3};
    double deviceXs[4] = {
        deviceNineRect.Left(),
        deviceNineP1.x,
        deviceNineP2.x,
        deviceNineRect.Right()};
    double deviceYs[4] = {
        deviceNineRect.Top(),
        deviceNineP1.y,
        deviceNineP2.y,
        deviceNineRect.Bottom()};
    double deviceScale = deviceBorderRectangle.Width() / screenBorderBounds.Width();
    
    double nineBackgroundLeft = deviceBorderRectangle.Left() - deviceNineRect.Left() ;
    double nineBackgroundTop = deviceBorderRectangle.Top() - deviceNineRect.Top() ;
    double nineBackgroundRight = deviceBorderRectangle.Right() - deviceNineP2.x + nineXs[2];
    double nineBackgroundBottom = deviceBorderRectangle.Bottom() - deviceNineP2.y + nineYs[2];

    Lv2cImageSurface shadowSurface{
        cairo_format_t::CAIRO_FORMAT_A8,
        (int)(nineXs[3]),
        (int)(nineYs[3])};

    // draw the background shape.
    Lv2cRoundCorners deviceRoundCorners = roundCorners * deviceScale;

    Lv2cDrawingContext bdc{shadowSurface};

    bdc.set_source(Lv2cColor(1, 1, 1));
    if (deviceRoundCorners.is_empty())
    {
            bdc.rectangle(
                Lv2cRectangle(
                    nineBackgroundLeft, nineBackgroundTop,
                    nineBackgroundRight - nineBackgroundLeft,
                    nineBackgroundBottom - nineBackgroundTop)
                    );

    } else {
    bdc.round_corner_rectangle(
        Lv2cRectangle(
            nineBackgroundLeft, nineBackgroundTop,
            nineBackgroundRight - nineBackgroundLeft,
            nineBackgroundBottom - nineBackgroundTop),
        deviceRoundCorners);
    }
    bdc.fill();
    cairo_surface_flush(shadowSurface.get());

    double xOffset, yOffset;

    (void)xOffset;
    (void)yOffset;
    BlurDropShadow(bdc, shadowSurface.get(), &xOffset, &yOffset);

    shadowSurface.mark_dirty();

    Lv2cImageSurface colorSurface{
        cairo_format_t::CAIRO_FORMAT_ARGB32,
        shadowSurface.get_width(),
        shadowSurface.get_height()};

    // create an argb surface from the a-only shadowSurface.
    Lv2cDrawingContext bdcColor{colorSurface};
    bdcColor.set_source(Lv2cColor(ShadowColor(),ShadowOpacity()));
    bdcColor.mask_surface(shadowSurface, 0, 0);

    // bdcColor.move_to(0,0);
    // bdcColor.line_to(shadowSurface.get_width(),shadowSurface.get_height());
    // bdcColor.set_source(Lv2cColor(1,0,0));
    // bdcColor.set_line_width(3);
    // bdcColor.stroke();

    colorSurface.mark_dirty();
    cairo_surface_mark_dirty(colorSurface.get());

    // debug: show the ninepatch.
    // {
    //     (void)deviceXs;
    //     (void)deviceYs;

    //     dc.save();
    //     dc.scale(deviceScale,deviceScale);
    //     dc.translate(deviceBorderRectangle.Left(),deviceBorderRectangle.Top());
    //     Lv2cPattern sourcePattern{colorSurface};
    //     Lv2cMatrix matrix;
    //     //matrix.translate(deviceBorderRectangle.Left(),deviceBorderRectangle.Top());
    //     //sourcePattern.set_matrix(matrix);

    //     dc.set_source(sourcePattern);
    //     //dc.set_source(Lv2cColor(1,0.5,0.5));

    //     dc.rectangle(
    //         0,
    //         0,
    //         nineXs[3],
    //         nineYs[3]);
    //     dc.fill();
    //     dc.restore();
    // }
    for (int ix = 0; ix < 3; ++ix)
    {
        for (int iy = 0; iy < 3; ++iy)
        {
            if (ix != 1 || iy != 1)
            {
                dc.save();
                dc.scale(1/deviceScale,1/deviceScale);
                dc.translate(deviceXs[ix] + xOffset,deviceYs[iy]+yOffset);
                Lv2cPattern sourcePattern{colorSurface};
                Lv2cMatrix matrix;
                matrix.translate(nineXs[ix],nineYs[iy]);
                matrix.scale(
                    (nineXs[ix+1]-nineXs[ix])/(deviceXs[ix+1]-deviceXs[ix]),
                    (nineYs[iy+1]-nineYs[iy])/(deviceYs[iy+1]-deviceYs[iy])
                );

                sourcePattern.set_matrix(matrix);

                dc.set_source(sourcePattern);
                //dc.set_source(Lv2cColor(1,0.5,0.5));

                dc.rectangle(
                    0,
                    0,
                    deviceXs[ix+1]-deviceXs[ix],
                    deviceYs[iy+1]-deviceYs[iy]);
                dc.fill();
                dc.restore();
            }
        }
    }
    super::DrawPostOpacity(dc, clipBounds);
    return true;
}

void Lv2cDropShadowElement::DrawDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{
    // extra margins needed in the buffer outside the clip region.
    double sourceLeft = clipBounds.Left() - this->XOffset() - Radius();
    double sourceRight = clipBounds.Right() - this->XOffset() + Radius();
    double sourceTop = clipBounds.Top() - this->YOffset() - Radius();
    double sourceBottom = clipBounds.Bottom() - this->YOffset() + Radius();

    Lv2cRectangle sourceBounds{sourceLeft, sourceTop, sourceRight - sourceLeft, sourceBottom - sourceTop};
    sourceBounds = ScreenClientBounds().Intersect(sourceBounds);
    if (sourceBounds.Empty())
    {
        return;
    }
    sourceBounds = sourceBounds.Inflate(Radius());
    Lv2cRectangle deviceBufferBounds = dc.user_to_device(sourceBounds).Ceiling();
    Lv2cRectangle userBufferBounds = dc.device_to_user(deviceBufferBounds);

    Lv2cRectangle deviceDisplayBounds = dc.user_to_device(
                                               sourceBounds.Translate(this->XOffset(), this->YOffset()))
                                             .Ceiling();

    Lv2cRectangle userDisplayBounds = dc.device_to_user(deviceDisplayBounds);
    (void)userDisplayBounds;

    double windowScale = Window()->WindowScale();

    cairo_public cairo_surface_t *renderSurface = cairo_image_surface_create(
        cairo_format_t::CAIRO_FORMAT_A8,
        (int)std::round(deviceBufferBounds.Width()),
        (int)std::round(deviceBufferBounds.Height()));

    cleanup renderSurfaceX{
        [renderSurface]()
        {
            cairo_surface_destroy(renderSurface);
        }};

    {
        Lv2cDrawingContext bdc(renderSurface);

        bdc.save();
        bdc.scale(windowScale, windowScale);
        bdc.translate(-userBufferBounds.Left(), -userBufferBounds.Top());
        super::DrawPostOpacity(bdc, userBufferBounds);
        bdc.restore();
    }
    cairo_surface_flush(renderSurface);
    double xOffset, yOffset;
    BlurDropShadow(dc, renderSurface, &xOffset, &yOffset);

    cairo_surface_mark_dirty(renderSurface);
    dc.save();
    {
        dc.set_source(Lv2cColor(ShadowColor(), ShadowOpacity()));
        dc.translate(userBufferBounds.Left() + xOffset, userBufferBounds.Top() + yOffset);
        dc.scale(1 / windowScale, 1 / windowScale);
        dc.mask_surface(renderSurface, 0, 0);
    }
    dc.restore();
    super::DrawPostOpacity(dc, clipBounds);
}

void Lv2cDropShadowElement::DrawInsetDropShadow(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds_)
{
    Lv2cRectangle clipBounds = clipBounds_.Intersect(ScreenClientBounds());
    if (clipBounds.Empty())
    {
        return;
    }
    double windowScale = Window()->WindowScale();

    // extra margins needed in the buffer outside the clip region.
    double leftExtra = std::max(this->XOffset() + Radius(), 0.0);
    double rightExtra = std::max(-this->XOffset() + Radius(), 0.0);
    double topExtra = std::max(this->YOffset() + Radius(), 0.0);
    double bottomExtra = std::max(-this->YOffset() + Radius(), 0.0);

    Lv2cRectangle clientBounds = ScreenClientBounds();
    Lv2cRectangle bounds =
        clientBounds.Intersect(
            clipBounds.Inflate(leftExtra, rightExtra, topExtra, bottomExtra));

    if (bounds.Empty())
        return;
    Lv2cRectangle deviceBufferBounds = dc.user_to_device(bounds).Ceiling();
    Lv2cRectangle deviceDisplayBounds = dc.user_to_device(clipBounds).Ceiling();

    Lv2cRectangle userBufferBounds = dc.device_to_user(deviceBufferBounds);
    Lv2cRectangle userDisplayBounds = dc.device_to_user(deviceDisplayBounds);

    Lv2cImageSurface colorSurface {
        cairo_format_t::CAIRO_FORMAT_ARGB32,
        (int)deviceBufferBounds.Width(), (int)deviceBufferBounds.Height()
    };
    // Render into the working buffer.
    Lv2cDrawingContext cdc(colorSurface);
    {

        cdc.save();
        cdc.scale(windowScale, windowScale);
        cdc.translate(-userBufferBounds.Left(), -userBufferBounds.Top());
        super::DrawPostOpacity(cdc, userBufferBounds);
        cdc.restore();
    }
    colorSurface.flush();

    Lv2cImageSurface alphaSurface { 
        cairo_format_t::CAIRO_FORMAT_A8,
        colorSurface.get_width(), colorSurface.get_height()
        };
    {
        Lv2cDrawingContext alphaDc(alphaSurface);
        alphaDc.set_operator(cairo_operator_t::CAIRO_OPERATOR_SOURCE);
        alphaDc.set_source(colorSurface,0,0);
        alphaDc.rectangle(0,0,alphaSurface.get_width(),alphaSurface.get_height());
        alphaDc.fill();
    }

    alphaSurface.flush();

    BlurInsetDropShadow(dc, alphaSurface.get());
    alphaSurface.mark_dirty();
    // Render the inset shadow ATOP the original.
    {
        cdc.set_source(Lv2cColor(ShadowColor(),ShadowOpacity()));
        cdc.set_operator(cairo_operator_t::CAIRO_OPERATOR_ATOP);
        cdc.mask_surface(alphaSurface,0,0);
        colorSurface.flush();
    }

    // Combine the results with the output buffer.
    dc.save();
    {
        dc.rectangle(userDisplayBounds);
        dc.clip();

        dc.set_source(colorSurface,0,0);

        dc.translate(userBufferBounds.Left(), userBufferBounds.Top());
        dc.scale(1 / windowScale, 1 / windowScale);
        dc.set_source(colorSurface,0,0);

        dc.rectangle(0,0,deviceBufferBounds.Width(),deviceBufferBounds.Height());
        dc.fill();
    }
    dc.restore();
}

void Lv2cDropShadowElement::DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{
    switch (this->Variant())
    {
    case Lv2cDropShadowVariant::DropShadow:
        // todo: check for solid background.
        // todo: if it is, make sure the clipBounds extends into the shadow area.
        if (IsInterior(clipBounds)) // solid, and the invlidation rectangle doesn't overlap the shadow?
        {
            super::DrawPostOpacity(dc, clipBounds);
        }
        else
        {
            if (!DrawFastDropShadow(dc, clipBounds))
            {
                DrawDropShadow(dc, clipBounds);
            }
        }
        break;
    case Lv2cDropShadowVariant::InnerDropShadow:
        DrawInsetDropShadow(dc, clipBounds);
        break;
    case Lv2cDropShadowVariant::Empty:
        super::DrawPostOpacity(dc, clipBounds);
        break;
    }
}

void Lv2cDropShadowElement::InvalidateScreenRect(const Lv2cRectangle &screenRectangle)
{
    if (Variant() == Lv2cDropShadowVariant::DropShadow)
    {
        int iRadius = (int)std::ceil(Radius());
        Lv2cRectangle withShadow = screenRectangle.Inflate(iRadius);
        super::InvalidateScreenRect(withShadow);
    }
    else
    {
        super::InvalidateScreenRect(screenRectangle);
    }
}
Lv2cDropShadowElement &Lv2cDropShadowElement::DropShadow(const Lv2cDropShadow &dropShadow)
{
    Variant(dropShadow.variant);
    XOffset(dropShadow.xOffset);
    YOffset(dropShadow.yOffset);
    Radius(dropShadow.radius);
    ShadowOpacity(dropShadow.opacity);
    ShadowColor(dropShadow.color);
    return *this;
}
Lv2cDropShadow Lv2cDropShadowElement::DropShadow() const
{
    return Lv2cDropShadow{
        .variant = Variant(),
        .xOffset = XOffset(),
        .yOffset = YOffset(),
        .radius = Radius(),
        .opacity = ShadowOpacity(),
        .color = ShadowColor()};
}

bool Lv2cDropShadowElement::IsSolidBackground() const
{
    auto &background = Style().Background();
    if (background.isEmpty())
        return false;
    if (background.get_type() != cairo_pattern_type_t::CAIRO_PATTERN_TYPE_SOLID)
    {
        return false;
    }
    Lv2cColor color = background.get_color();
    return color.A() == 1.0;
}
bool Lv2cDropShadowElement::IsInterior(const Lv2cRectangle &rectangle) const
{
    if (!IsSolidBackground())
        return false;
    auto roundCorners = this->Style().RoundCorners().PixelValue();

    double left = screenBorderBounds.Left() + std::max(roundCorners.topLeft, roundCorners.bottomLeft);
    double right = screenBorderBounds.Right() - std::max(roundCorners.topRight, roundCorners.bottomRight);
    double top = screenBorderBounds.Top() + std::max(roundCorners.topLeft, roundCorners.topRight);
    double bottom = screenBorderBounds.Bottom() - std::max(roundCorners.bottomLeft, roundCorners.bottomRight);
    return rectangle.Left() >= left && rectangle.Right() <= right && rectangle.Top() >= top && rectangle.Bottom() <= bottom;
}

void Lv2cDropShadowElement::FinalizeLayout(const Lv2cRectangle &layoutClipRect, const Lv2cRectangle &parentBounds, bool clippedInLayout)
{
    super::FinalizeLayout(layoutClipRect, parentBounds, clippedInLayout);
    if (!this->clippedInLayout && !this->clientBounds.Empty())
    {
        if (this->Variant() == Lv2cDropShadowVariant::DropShadow)
        {
            Lv2cRectangle shadowRect = this->clientBounds.Translate(XOffset(), YOffset()).Inflate(Radius());
            this->screenDrawBounds = screenDrawBounds.Union(shadowRect);
        }
    }
}
