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

#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkLog.hpp"
#include <numbers>
#include "ss.hpp"
#include "lvtk/LvtkSvg.hpp"

using namespace lvtk;

void LvtkSvgElement::Load()
{
    if (changed)
    {
        try
        {
            changed = false;
            this->image = Window()->GetSvgImage(Source());
        }
        catch (const std::exception &e)
        {
            LogError(SS("Can't load svg file. " << e.what()));
        }
    }
}

LvtkSize LvtkSvgElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context)
{
    LvtkSize result {clientConstraint};
    if (clientConstraint.Width() == 0 || clientConstraint.Height() == 0)
    {
        LvtkSize imageSize {24,24};
        if (image)
        {
            imageSize = image->intrinsic_size();
        } 
        if (clientConstraint.Width() == 0 && clientConstraint.Height() == 0)
        {
            result = imageSize;
        } else if (clientConstraint.Width() == 0)
        {
            result = LvtkSize(
                result.Height()*imageSize.Width()/imageSize.Height(),
                result.Height());
        } else { //if (clientConstraint.getHeight() == 0)
            result = LvtkSize(
                result.Width(),
                result.Width()*imageSize.Height()/imageSize.Width());

        }
        result = imageSize;
    } 
    this->measuredImageSize = result;
    return result;
}



void LvtkSvgElement::LvtkSvgElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    LvtkSize size = measuredImageSize;

    LvtkRectangle imageBounds(
        0,0,
        size.Width(),
        size.Height());

    double rotation = Rotation();
    if (image)
    {
        LvtkPattern tintColor = Style().TintColor();
        if (rotation != 0)
        {
            dc.save();
            dc.translate(size.Width()/2,size.Height()/2);
            dc.rotate(rotation*std::numbers::pi/180.0);;
            dc.translate(-size.Width()/2,-size.Height()/2);
        }

        if (tintColor.isEmpty())
        {
            image->render(dc,imageBounds);
        } else {
            if (dc.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
            {
                throw std::runtime_error(SS("Lvtk: " << LvtkStatusMessage(dc.status())));
            }
            dc.push_group();
            if (dc.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
            {
                throw std::runtime_error(SS("Lvtk: " << LvtkStatusMessage(dc.status())));
            }
            image->render(dc,imageBounds);
            LvtkPattern p = dc.pop_group();
            dc.check_status();
            dc.set_source(tintColor);
            dc.mask(p);
        

        }
        if (rotation != 0)
        {
            dc.restore();
        }
    } else {
        // gray marker if no image.
        dc.set_source(LvtkColor(0.5,0.5,0.5,0.25));
        dc.rectangle(imageBounds);
        dc.fill();
    }

}
void LvtkSvgElement::OnMount()
{
    super::OnMount();
    if (changed)
    {
        Load();
    }
}


LvtkSvgElement&LvtkSvgElement::Rotation(double angle)
{
    RotationProperty.set(angle);
    return *this;
}
double LvtkSvgElement::Rotation() const
{
    return RotationProperty.get();
}

void LvtkSvgElement::OnRotationChanged(double value)
{
    Invalidate();
}
void LvtkSvgElement::OnSourceChanged(const std::string&value)
{
    this->changed = true;
    if (IsMounted())
    {
        Load();
        Invalidate();
    }
}

LvtkSvgElement &LvtkSvgElement::Source(const std::string &source)
{
    SourceProperty.set(source);
    return *this;
}
const std::string &LvtkSvgElement::Source() const
{
    return SourceProperty.get();
}


LvtkSvgElement::LvtkSvgElement()
{

    rotationObserverHandle = RotationProperty.addObserver([this](double value) {
        OnRotationChanged(value);
    });

    sourceObserverHandle = SourceProperty.addObserver([this](const std::string&value) {
        OnSourceChanged(value);
    });
}