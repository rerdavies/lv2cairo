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

#include "lv2c/Lv2cPngElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cLog.hpp"
#include <numbers>
#include "ss.hpp"
// #include "lv2c/lv2tk_png.hpp"

using namespace lv2c;

void Lv2cPngElement::Load()
{
    if (changed)
    {
        try
        {
            changed = false;
            this->surface = Window()->GetPngImage(Source());
        }
        catch (const std::exception &e)
        {
            LogError(SS("Can't load png file. " << e.what()));
        }
    }
}

Lv2cSize Lv2cPngElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
{
    Lv2cSize result{clientConstraint};
    if (clientConstraint.Width() == 0 || clientConstraint.Height() == 0)
    {
        Lv2cSize imageSize{24, 24};
        if (surface)
        {
            imageSize = surface.size();
        }
        if (clientConstraint.Width() == 0 && clientConstraint.Height() == 0)
        {
            result = imageSize;
        }
        else if (clientConstraint.Width() == 0)
        {
            result = Lv2cSize(
                result.Height() * imageSize.Width() / imageSize.Height(),
                result.Height());
        }
        else
        { // if (clientConstraint.getHeight() == 0)
            result = Lv2cSize(
                result.Width(),
                result.Width() * imageSize.Height() / imageSize.Width());
        }
        result = imageSize;
    }
    this->measuredImageSize = result;
    return result;
}

static inline Lv2cRectangle FillRect(
    const Lv2cRectangle &clientBounds,
    Lv2cSize imageSize)
{
    double clientWidth = clientBounds.Width();
    double clientHeight = clientBounds.Height();
    double widthRatio = clientWidth / imageSize.Width();
    double heightRatio = clientHeight / imageSize.Height();

    if (widthRatio == heightRatio)
    {
        return clientBounds;
    }

    if (widthRatio > heightRatio)
    {
        // fit to width.
        double height =
            clientBounds.Width() * imageSize.Height() / imageSize.Width();
        double heightExtra = height - clientBounds.Height();
        return Lv2cRectangle(
            clientBounds.Left(),
            clientBounds.Top() - heightExtra * 0.5,
            clientBounds.Width(),
            clientBounds.Height() + heightExtra);
    }
    else
    {
        // fit to height.
        double width = clientBounds.Height() * imageSize.Width() / imageSize.Height();
        double widthExtra = width - clientBounds.Width();
        return Lv2cRectangle(
            clientBounds.Left() - widthExtra * 0.5,
            clientBounds.Top(),
            clientBounds.Width() + widthExtra,
            clientBounds.Height());
    }
}
static inline Lv2cRectangle FitRect(
    const Lv2cRectangle &clientBounds,
    Lv2cSize imageSize)
{
    double clientWidth = clientBounds.Width();
    double clientHeight = clientBounds.Height();
    double widthRatio = clientWidth / imageSize.Width();
    double heightRatio = clientHeight / imageSize.Height();
    if (widthRatio == heightRatio)
    {
        return clientBounds;
    }
    if (widthRatio < heightRatio)
    {
        // fit to width.
        double height =
            clientBounds.Width() * imageSize.Height() / imageSize.Width();
        double heightExtra = height - clientBounds.Height();
        return Lv2cRectangle(
            clientBounds.Left(),
            clientBounds.Top() - heightExtra * 0.5,
            clientBounds.Width(),
            clientBounds.Height() + heightExtra);
    }
    else
    {
        // fit to height.
        double width = clientBounds.Height() * imageSize.Width() / imageSize.Height();
        double widthExtra = width - clientBounds.Width();
        return Lv2cRectangle(
            clientBounds.Left() - widthExtra * 0.5,
            clientBounds.Top(),
            clientBounds.Width() + widthExtra,
            clientBounds.Height());
    }
}

void Lv2cPngElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    Lv2cSize clientSize = ClientSize();

    Lv2cRectangle imageBounds(0, 0,
                               clientSize.Width(),
                               clientSize.Height());
    if (imageBounds.Empty())
    {
        return;
    }
    if (!surface)
    {
        // gray marker if no image.
        dc.set_source(Lv2cColor(0.5, 0.5, 0.5, 0.25));
        dc.rectangle(imageBounds);
        dc.fill();
        return;
    }
    Lv2cSize imageSize = surface.size();
    if (imageSize.Width() <= 0 || imageSize.Height() <= 0)
    {
        return;
    }

    switch (ImageAlignment())
    {
    case Lv2cImageAlignment::Stretch:
        imageBounds = Lv2cRectangle(0, 0, clientSize.Width(), clientSize.Height());
        break;
    case Lv2cImageAlignment::Fill:
        imageBounds = FillRect(imageBounds, imageSize);
        break;
    case Lv2cImageAlignment::Fit:
        imageBounds = FitRect(imageBounds, imageSize);
        break;
    }

    double rotation = Rotation();

    Lv2cPattern tintColor = Style().TintColor();
    if (rotation != 0)
    {
        dc.save();
        dc.translate(clientSize.Width() / 2, clientSize.Height() / 2);
        dc.rotate(rotation * std::numbers::pi / 180.0);
        ;
        dc.translate(-clientSize.Width() / 2, -clientSize.Height() / 2);
    }


    auto savedOperator = dc.get_operator();
    dc.save();
    if (ImageAlignment() == Lv2cImageAlignment::Fill)
    {
        dc.rectangle(Lv2cRectangle(0, 0, clientSize.Width(), clientSize.Height()));
        dc.clip();
    }
    dc.set_operator(cairo_operator_t::CAIRO_OPERATOR_OVER);
    dc.rectangle(imageBounds);
    dc.translate(imageBounds.Left(), imageBounds.Top());
    dc.scale(imageBounds.Width() / imageSize.Width(), imageBounds.Height() / imageSize.Height());
    dc.set_source(surface, 0, 0);

    dc.fill();
    dc.set_operator(savedOperator);
    dc.restore();
    if (rotation != 0)
    {
        dc.restore();
    }
}
void Lv2cPngElement::OnMount()
{
    super::OnMount();
    if (changed)
    {
        Load();
    }
}

Lv2cPngElement &Lv2cPngElement::Rotation(double angle)
{
    RotationProperty.set(angle);
    return *this;
}
double Lv2cPngElement::Rotation() const
{
    return RotationProperty.get();
}

void Lv2cPngElement::OnRotationChanged(double value)
{
    Invalidate();
}
void Lv2cPngElement::OnSourceChanged(const std::string &value)
{
    this->changed = true;
    if (IsMounted())
    {
        Load();
        Invalidate();
    }
}

Lv2cPngElement &Lv2cPngElement::Source(const std::string &source)
{
    SourceProperty.set(source);
    return *this;
}
const std::string &Lv2cPngElement::Source() const
{
    return SourceProperty.get();
}

Lv2cPngElement::Lv2cPngElement()
{
    RotationProperty.SetElement(this, &Lv2cPngElement::OnRotationChanged);
    SourceProperty.SetElement(this, &Lv2cPngElement::OnSourceChanged);
    ImageAlignmentProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}

Lv2cPngElement &Lv2cPngElement::ImageAlignment(Lv2cImageAlignment value)
{
    ImageAlignmentProperty.set(value);
    return *this;
}
Lv2cImageAlignment Lv2cPngElement::ImageAlignment() const
{
    return ImageAlignmentProperty.get();
}
