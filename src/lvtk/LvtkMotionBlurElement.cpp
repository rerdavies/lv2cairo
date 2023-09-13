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

#include "lvtk/LvtkMotionBlurElement.hpp"
#include "lvtk/LvtkDrawingContext.hpp"
#include "lvtk/LvtkWindow.hpp"
#include <cmath>
#include <cassert>

using namespace lvtk;

LvtkMotionBlurElement::LvtkMotionBlurElement()
{
    FromProperty.SetElement(this, LvtkBindingFlags::InvalidateLayoutOnChanged);
    ToProperty.SetElement(this, LvtkBindingFlags::InvalidateLayoutOnChanged);
}

bool LvtkMotionBlurElement::WillDraw() const
{
    return true;
}

LvtkImageSurface LvtkMotionBlurElement::MotionBlurFilter(LvtkImageSurface &surface, LvtkPoint from, LvtkPoint to)
{
    surface.flush();

    int sourceWidth = surface.get_width();
    int sourceHeight = surface.get_height();
    int sourceStride = surface.get_stride();
    LvtkImageSurface result{cairo_format_t::CAIRO_FORMAT_ARGB32, sourceWidth, sourceHeight};

    uint8_t *sourceData = surface.get_data();
    uint8_t *destData = result.get_data();

        //std::cout << "from.y: " << from.y <<  " to.y: " << to.y << std::endl;


    if (from.x == to.x)
    {
        if (std::abs(to.y - from.y) <= 1)
        {
            // linear blend.

            double yVal = to.y;
            (void)yVal;
            
            std::vector<LvtkLinearColor> line0Buffer;
            line0Buffer.resize(sourceWidth);
            std::vector<LvtkLinearColor> line1Buffer;
            line1Buffer.resize(sourceWidth);

            std::vector<LvtkLinearColor> lineResult;
            lineResult.resize(sourceWidth);

            double blend0 = (from.y - std::floor(from.y));
            //std::cout << "blend0: " << blend0 << std::endl;
            double blend1 = 1 - blend0;
            int iy = std::floor(from.y);
            for (int y = 0; y < sourceHeight; ++y)
            {
                int y0Source = y - iy-1;
                int y1Source = y - iy ;
                                uint8_t *pSource0 = sourceData + (sourceStride * y0Source);
                uint8_t *pSource1 = pSource0 + sourceStride;

                if (y0Source >= 0 && y0Source < sourceHeight)
                {
                    LvtkLinearColor::FromImageSurface(pSource0, line0Buffer);
                    if (y1Source >= 0 && y1Source < sourceHeight)
                    {
                        // both lines good.
                        LvtkLinearColor::FromImageSurface(pSource1, line1Buffer);
                        for (int x = 0; x < sourceWidth; ++x)
                        {
                            lineResult[x] = line0Buffer[x] * blend0 + line1Buffer[x] * blend1;
                        }
                    }
                    else
                    {
                        // first line good.
                        for (int x = 0; x < sourceWidth; ++x)
                        {
                            lineResult[x] = line0Buffer[x] * blend0;
                        }
                    }
                }
                else
                {
                    if (y1Source >= 0 && y1Source < sourceHeight)
                    {
                        // second line good.
                        LvtkLinearColor::FromImageSurface(pSource1, line1Buffer);
                        for (int x = 0; x < sourceWidth; ++x)
                        {
                            lineResult[x] = line1Buffer[x] * blend1;
                        }
                    }
                    else
                    {
                        // neithter valid.
                        for (int x = 0; x < sourceWidth; ++x)
                        {
                            lineResult[x] = LvtkLinearColor();
                        }
                    }
                }
                unsigned char *pDest = destData + y * sourceStride;
                LvtkLinearColor::ToImageSurface(lineResult, pDest);
            }
        }
        else
        {
            if (to.y < from.y)
            {
                std::swap(from, to);
            }

            int yFrom = (int)std::round(from.y);
            int yTo = (int)std::round(to.y);
            if (yTo == yFrom)
            {
                yTo = yFrom + 1;
            }
            int bufferSize = (yTo - yFrom);

            std::vector<LvtkLinearColor> lineBufferMemory;
            lineBufferMemory.resize(sourceWidth * bufferSize);

            std::vector<LvtkLinearColor> currentLine;

            auto GetBufferLine = [&lineBufferMemory, sourceWidth, bufferSize](int y)
            {
                y = y % (int)bufferSize;
                if (y < 0)
                    y += bufferSize;
                assert(y < (int)bufferSize);
                assert((y + 1) * sourceWidth <= (int)(lineBufferMemory.size()));
                return &(lineBufferMemory[sourceWidth * y]);
            };
            std::vector<LvtkLinearColor> runningLine;
            runningLine.resize(sourceWidth);

            // Calculate lead-in lines.
            if (yFrom < 0)
            {
                for (int y = -bufferSize; y < 0; ++y)
                {
                    auto bufferLine = GetBufferLine(y + bufferSize);
                    int ySource = (int)(y - yFrom);

                    if (ySource >= 0 && ySource < sourceHeight)
                    {
                        uint8_t *pSource = sourceData + (sourceStride * ySource);
                        for (int x = 0; x < sourceWidth; ++x)
                        {
                            LvtkLinearColor c = LvtkLinearColor::FromImageSurfaceColor(pSource[2], pSource[1], pSource[0], pSource[3]);
                            bufferLine[x] = c;
                            runningLine[x] += c;
                            pSource += 4;
                        }
                    }
                }
            }

            float scale = 1.0f / (float)bufferSize;
            (void)scale;
            for (int y = 0; y < sourceHeight; ++y)
            {
                int ySource = (int)(y - yTo);
                auto bufferLine = GetBufferLine(y);
                for (int x = 0; x < sourceWidth; ++x)
                {
                    runningLine[x] -= bufferLine[x];
                }
                if (ySource >= 0 && ySource < sourceHeight)
                {
                    uint8_t *pSource = sourceData + (sourceStride * ySource);
                    for (int x = 0; x < sourceWidth; ++x)
                    {
                        LvtkLinearColor c = LvtkLinearColor::FromImageSurfaceColor(
                            (uint8_t)(pSource[2]),
                            (uint8_t)(pSource[1]),
                            (uint8_t)(pSource[0]),
                            (uint8_t(pSource[3])));
                        bufferLine[x] = c;
                        runningLine[x] += c;
                        pSource += 4;
                    }
                }
                else
                {
                    for (int x = 0; x < sourceWidth; ++x)
                    {
                        bufferLine[x] = LvtkLinearColor();
                    }
                }

                uint8_t *pDest = destData + sourceStride * y;
                LvtkLinearColor::ToImageSurface((size_t)sourceWidth, &(runningLine[0]), pDest, scale);

                assert(result.get_stride() == surface.get_stride());

                // Memory is in BGRA order.
                // pDest = destData + sourceStride*y;
                //  for (size_t x = 0; x < sourceWidth; ++x)
                //  {
                //      pDest[0] = 0x80; //B
                //      pDest[1] = 0x0; //G
                //      pDest[2] = 0x0;  //R
                //      pDest[3] = 0x80; //A
                //      pDest += 4;
                //  }
                //  assert (result.get_stride() == surface.get_stride());
            }
        }
        result.mark_dirty();
        return result;
    }
    else if (from.y == to.y)
    {
        int xFrom = (int)std::round(from.x);
        int xTo = (int)std::round(to.x);

        if (xFrom == xTo)
        {
            xTo = xFrom + 1;
        }
        if (xFrom > xTo)
        {
            std::swap(xFrom, xTo);
        }

        int bufferSize = (size_t)(xTo - xFrom);

        std::vector<LvtkLinearColor> columnBufferMemory;
        columnBufferMemory.resize(bufferSize);

        float scale = 1.0f / (float)bufferSize;

        std::vector<LvtkLinearColor> currentLine;
        currentLine.resize(sourceWidth);

        for (int y = 0; y < sourceHeight; ++y)
        {

            // zero out the buffer.
            columnBufferMemory.resize(0);
            columnBufferMemory.resize(bufferSize);

            LvtkLinearColor runningColor;

            uint8_t *sourceLine = sourceData + sourceStride * y;

            // Calculate lead-in columns
            for (int x = -bufferSize; x < 0; ++x)
            {
                int xSource = (int)(x - xFrom);

                if (xSource >= 0 && xSource < sourceWidth)
                {
                    uint8_t *pSource = sourceLine + xSource * 4;
                    LvtkLinearColor c = LvtkLinearColor::FromImageSurfaceColor(pSource[2], pSource[1], pSource[0], pSource[3]);
                    int bufferX = (x + bufferSize) % bufferSize;

                    columnBufferMemory[bufferX] = c;
                    runningColor += c;
                }
            }

            uint8_t *pSource = sourceLine - xFrom * 4;

            for (int x = 0; x < sourceWidth; ++x)
            {
                int xSource = (int)(x - xFrom);
                auto &bufferX = columnBufferMemory[x % bufferSize];
                runningColor -= bufferX;
                if (xSource >= 0 && xSource < sourceWidth)
                {
                    LvtkLinearColor c = LvtkLinearColor::FromImageSurfaceColor(
                        (uint8_t)(pSource[2]),
                        (uint8_t)(pSource[1]),
                        (uint8_t)(pSource[0]),
                        (uint8_t(pSource[3])));
                    bufferX = c;
                    runningColor += c;
                }
                else
                {
                    bufferX = LvtkLinearColor();
                }
                pSource += 4;

                currentLine[x] = runningColor;
            }
            uint8_t *pDest = destData + sourceStride * y;
            LvtkLinearColor::ToImageSurface((size_t)sourceWidth, &(currentLine[0]), pDest, scale);
        }
        result.mark_dirty();
        return result;
    }
    else
    {
        throw std::runtime_error("Not supported. Blur must be either horizontal or vertical");
    }
}

void LvtkMotionBlurElement::DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds)
{
    if (From() == LvtkPoint(0, 0) && To() == LvtkPoint(0, 0))
    {
        super::DrawPostOpacity(dc, clipBounds);
        return;
    }
    if (From() == To())
    {
        LvtkRectangle translatedBounds = this->ScreenBounds().Translate(To().x, To().y);
        LvtkRectangle clip = clipBounds.Intersect(this->ScreenBounds());
        LvtkRectangle translatedClip = clip.Intersect(translatedBounds);
        if (translatedClip.Empty())
        {
            return;
        }
        {
            dc.save();
            dc.rectangle(translatedClip);
            dc.clip();
            dc.translate(To().x, To().y);
            super::DrawPostOpacity(dc, translatedClip.Translate(-To().x, -To().y));

            dc.restore();
        }
        return;
    }
    // TODO: Don't draw ENTIRE screenRect: respect the clipBounds.

    // Capture the contents rendered at device scale.
    LvtkRectangle boundsRect = this->ScreenBounds();
    if (boundsRect.Empty())
    {
        return;
    }

    LvtkRectangle deviceRectangle = dc.user_to_device(boundsRect).Ceiling();
    LvtkRectangle userRectangle = dc.device_to_user(deviceRectangle);

    LvtkImageSurface renderSurface{
        cairo_format_t::CAIRO_FORMAT_ARGB32,
        (int)std::round(deviceRectangle.Width()),
        (int)std::round(deviceRectangle.Height())};

    LvtkDrawingContext bufferDc(renderSurface);
    bufferDc.scale(deviceRectangle.Width() / userRectangle.Width(), deviceRectangle.Height() / userRectangle.Height());

    bufferDc.translate(-userRectangle.Left(), -userRectangle.Top());

    super::DrawPostOpacity(bufferDc, clipBounds);

    LvtkPoint deviceFrom = dc.device_to_user_distance(From());
    LvtkPoint deviceTo = dc.device_to_user_distance(To());

    LvtkSurface filteredSurface = MotionBlurFilter(renderSurface, deviceFrom, deviceTo);
    // Put the modified contents back.
    dc.save();
    {
        dc.translate(userRectangle.Left(), userRectangle.Top());
        dc.scale(userRectangle.Width() / deviceRectangle.Width(), userRectangle.Height() / deviceRectangle.Height());
        dc.rectangle(LvtkRectangle(0, 0, deviceRectangle.Width(), deviceRectangle.Height()));
        LvtkPattern pattern(filteredSurface);
        dc.set_source(pattern);
        dc.fill();
    }
    dc.restore();
}
