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

#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cLog.hpp"
#include <numbers>
#include "ss.hpp"
#include "lv2c/Lv2cSvg.hpp"

using namespace lvtk;

void Lv2cSvgElement::Load()
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

Lv2cSize Lv2cSvgElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context)
{
    Lv2cSize result {clientConstraint};
    if (clientConstraint.Width() == 0 || clientConstraint.Height() == 0)
    {
        Lv2cSize imageSize {24,24};
        if (image)
        {
            imageSize = image->intrinsic_size();
        } 
        if (clientConstraint.Width() == 0 && clientConstraint.Height() == 0)
        {
            result = imageSize;
        } else if (clientConstraint.Width() == 0)
        {
            result = Lv2cSize(
                result.Height()*imageSize.Width()/imageSize.Height(),
                result.Height());
        } else { //if (clientConstraint.getHeight() == 0)
            result = Lv2cSize(
                result.Width(),
                result.Width()*imageSize.Height()/imageSize.Width());

        }
        result = imageSize;
    } 
    this->measuredImageSize = result;
    return result;
}



void Lv2cSvgElement::Lv2cSvgElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    Lv2cSize size = measuredImageSize;

    Lv2cRectangle imageBounds(
        0,0,
        size.Width(),
        size.Height());

    double rotation = Rotation();
    if (image)
    {
        Lv2cPattern tintColor = Style().TintColor();
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
                throw std::runtime_error(SS("Lv2c: " << Lv2cStatusMessage(dc.status())));
            }
            dc.push_group();
            if (dc.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
            {
                throw std::runtime_error(SS("Lv2c: " << Lv2cStatusMessage(dc.status())));
            }
            image->render(dc,imageBounds);
            Lv2cPattern p = dc.pop_group();
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
        dc.set_source(Lv2cColor(0.5,0.5,0.5,0.25));
        dc.rectangle(imageBounds);
        dc.fill();
    }

}
void Lv2cSvgElement::OnMount()
{
    super::OnMount();
    if (changed)
    {
        Load();
    }
}


Lv2cSvgElement&Lv2cSvgElement::Rotation(double angle)
{
    RotationProperty.set(angle);
    return *this;
}
double Lv2cSvgElement::Rotation() const
{
    return RotationProperty.get();
}

void Lv2cSvgElement::OnRotationChanged(double value)
{
    Invalidate();
}
void Lv2cSvgElement::OnSourceChanged(const std::string&value)
{
    this->changed = true;
    if (IsMounted())
    {
        Load();
        Invalidate();
    }
}

Lv2cSvgElement &Lv2cSvgElement::Source(const std::string &source)
{
    SourceProperty.set(source);
    return *this;
}
const std::string &Lv2cSvgElement::Source() const
{
    return SourceProperty.get();
}


Lv2cSvgElement::Lv2cSvgElement()
{

    rotationObserverHandle = RotationProperty.addObserver([this](double value) {
        OnRotationChanged(value);
    });

    sourceObserverHandle = SourceProperty.addObserver([this](const std::string&value) {
        OnSourceChanged(value);
    });
}