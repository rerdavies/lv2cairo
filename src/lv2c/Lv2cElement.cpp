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

#include "lv2c/Lv2cElement.hpp"
#include "lv2c/Lv2cLog.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cTypes.hpp"
#include "lv2c/Lv2cContainerElement.hpp"
#include <stdexcept>
#include <iostream>
#include <numbers>
#include "cleanup.hpp"
#include "ss.hpp"

using namespace lv2c;

Lv2cElement::Lv2cElement()
{
    this->Style().SetElement(this);
}

Lv2cElement::~Lv2cElement() noexcept
{
}

// make sure sum of raddii on an edge don't exceed the length of the edge.
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

static double degreesToRadians(double angle)
{
    return angle * (std::numbers::pi / 180.0);
}

static void drawRoundRect(Lv2cDrawingContext &dc, const Lv2cRectangle &bounds, Lv2cRoundCorners corners)
{
    // reduce radii if neccessary.
    FitRadii(bounds.Width(), corners.topLeft, corners.topRight);
    FitRadii(bounds.Width(), corners.bottomLeft, corners.bottomRight);
    FitRadii(bounds.Height(), corners.topLeft, corners.bottomLeft);
    FitRadii(bounds.Height(), corners.topRight, corners.bottomRight);

    dc.move_to(bounds.Left() + corners.topLeft, bounds.Top());
    dc.arc(
        bounds.Right() - corners.topRight,
        bounds.Top() + corners.topRight,
        corners.topRight,
        degreesToRadians(-90),
        degreesToRadians(0));

    dc.arc(
        bounds.Right() - corners.bottomRight,
        bounds.Bottom() - corners.bottomRight,
        corners.bottomRight,
        degreesToRadians(0),
        degreesToRadians(90));

    dc.arc(
        bounds.Left() + corners.bottomLeft,
        bounds.Bottom() - corners.bottomLeft,
        corners.bottomLeft,
        degreesToRadians(90),
        degreesToRadians(180));

    dc.arc(
        bounds.Left() + corners.topLeft,
        bounds.Top() + corners.topLeft,
        corners.topLeft,
        degreesToRadians(180),
        degreesToRadians(270));
    dc.close_path();
}

void Lv2cElement::drawRoundBorderRect(Lv2cDrawingContext &dc)
{
    Lv2cRectangle borderBounds = this->borderBounds.Translate(
        -this->clientBounds.Left(),
        -this->clientBounds.Top());

    ::drawRoundRect(dc, borderBounds, this->roundCorners);
}

static void InsetCorner(double &corner, double xBorder, double yBorder)
{
    double inset = std::max(xBorder, yBorder);
    if (corner <= inset)
    {
        corner = 0;
    }
    else
    {
        corner -= inset;
    }
}
void Lv2cElement::drawRoundInsideBorderRect(Lv2cDrawingContext &dc)
{
    Lv2cRectangle borderInnerBounds = this->paddingBounds.Translate(
        -this->clientBounds.Left(),
        -this->clientBounds.Top());

    Lv2cRoundCorners corners = this->roundCorners;
    auto borderWidth = Style().BorderWidth();
    double leftBorder = borderWidth.Left().PixelValue();
    double rightBorder = borderWidth.Left().PixelValue();
    double topBorder = borderWidth.Top().PixelValue();
    double bottomBorder = borderWidth.Bottom().PixelValue();

    InsetCorner(corners.topLeft, leftBorder, topBorder);
    InsetCorner(corners.topRight, rightBorder, topBorder);
    InsetCorner(corners.bottomRight, rightBorder, bottomBorder);
    InsetCorner(corners.bottomLeft, leftBorder, bottomBorder);

    ::drawRoundRect(dc, borderInnerBounds, corners);
}

void Lv2cElement::OnDraw(Lv2cDrawingContext &dc)
{

    bool hasRoundCorners = !this->Style().RoundCorners().isEmpty();

    if (hasRoundCorners)
    {
        dc.save();
        drawRoundBorderRect(dc);
        dc.clip();

        if (!this->Style().Background().isEmpty())
        {
            Lv2cRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            dc.set_source(this->Style().Background());
            dc.rectangle(borderBounds);
            dc.fill();
        }
        if (this->WillDrawBorder())
        {

            Lv2cRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());

            dc.set_source(this->Style().BorderColor());

            dc.rectangle(borderBounds); // Safer to let the clip mask take care of the border bounds.
            drawRoundInsideBorderRect(dc);
            auto oldFill = dc.get_fill_rule();
            dc.set_fill_rule(cairo_fill_rule_t::CAIRO_FILL_RULE_EVEN_ODD);
            dc.fill();
            dc.set_fill_rule(oldFill);
        }

        dc.restore();
    }
    else
    {

        if (!this->Style().Background().isEmpty())
        {
            Lv2cRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            dc.set_source(this->Style().Background());
            dc.rectangle(borderBounds);
            dc.fill();
        }
        if (this->WillDrawBorder())
        {
            Lv2cRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            Lv2cRectangle borderInnerBounds = this->paddingBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            dc.set_source(this->Style().BorderColor());
            dc.rectangle(borderBounds);
            dc.rectangle(borderInnerBounds);
            auto oldFill = dc.get_fill_rule();
            dc.set_fill_rule(cairo_fill_rule_t::CAIRO_FILL_RULE_EVEN_ODD);
            dc.fill();
            dc.set_fill_rule(oldFill);
        }
    }
}

void Lv2cElement::OnMount()
{
}

void Lv2cElement::OnUnmount()
{
}
void Lv2cElement::OnMount(Lv2cWindow *window)
{
    OnMount();
    OnMounted.Fire(window);
}
void Lv2cElement::OnUnmount(Lv2cWindow *window)
{
    OnUnmounted.Fire(window);
    OnUnmount();


}

void Lv2cElement::Draw(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{
    if (clippedInLayout)
    {

        return;
    }

    dc.check_status();

    if (Style().Visibility() != Lv2cVisibility::Visible)
    {
        return;
    }
    if (Style().Opacity() != 1.0)
    {
        double windowScale = Window()->WindowScale();

        Lv2cRectangle bounds = clipBounds.Intersect(this->screenDrawBounds);
        if (bounds.Empty())
            return;
        Lv2cRectangle deviceBounds = dc.user_to_device(bounds);
        double left = std::floor(deviceBounds.Left());
        double top = std::floor(deviceBounds.Top());
        double right = std::ceil(deviceBounds.Right());
        double bottom = std::ceil(deviceBounds.Bottom());
        deviceBounds = Lv2cRectangle(left, top, right - left, bottom - top);

        Lv2cRectangle screenBounds = dc.device_to_user(deviceBounds);

        cairo_public cairo_surface_t *renderSurface = cairo_image_surface_create(
            cairo_format_t::CAIRO_FORMAT_ARGB32,
            (int)std::round(deviceBounds.Width()),
            (int)std::round(deviceBounds.Height()));
        {
            Lv2cDrawingContext bdc(renderSurface);

            bdc.save();
            bdc.scale(windowScale, windowScale);
            dc.set_operator(cairo_operator_t::CAIRO_OPERATOR_OVER);
            bdc.translate(-screenBounds.Left(), -screenBounds.Top());
            DrawPostOpacity(bdc, clipBounds);
            bdc.restore();
        }

        dc.save();
        {
            dc.rectangle(screenBounds);
            dc.translate(screenBounds.Left(), screenBounds.Top());
            dc.scale(1 / windowScale, 1 / windowScale);
            dc.set_source(renderSurface, 0, 0);
            double alpha = Style().Opacity();
            alpha = pow(alpha, 2.2);
            dc.set_operator(cairo_operator_t::CAIRO_OPERATOR_OVER);
            dc.paint_with_alpha(alpha);
            // dc.mask_surface(renderSurface, 0, 0);
        }
        dc.restore();

        cairo_surface_destroy(renderSurface);

        dc.check_status();
    }
    else
    {
        DrawPostOpacity(dc, clipBounds);
        if (dc.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            LogError(SS("Drawing error: " << Lv2cStatusMessage(dc.status())));
        }
    }
}
void Lv2cElement::DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds)
{
    if (!clipBounds.Intersects(this->screenDrawBounds))
        return;

    if (this->Style().Visibility() != Lv2cVisibility::Visible)
    {
        return;
    }

    // std::cout
    //     << "  draw: " << screenBorderBounds.getLeft()
    //     << "," << screenBorderBounds.getTop()
    //     << "," << screenBorderBounds.getRight()
    //     << "," << screenBorderBounds.getBottom()
    //     << std::endl;
    if (screenBorderBounds.Width() <= 0 || screenBorderBounds.Height() < 0)
        return;
    if (WillDraw())
    {
        Lv2cRectangle clipRect = screenBorderBounds.Intersect(clipBounds);
        if (!clipRect.Empty())
        {
            dc.save();

            dc.rectangle(clipRect);
            dc.clip();
            dc.translate(screenClientBounds.Left(), screenClientBounds.Top());

            OnDraw(dc);
            dc.restore();
        }
    }
    if (WillDrawOver())
    {
        Lv2cRectangle clipRect = screenBorderBounds.Intersect(clipBounds);
        if (!clipRect.Empty())
        {
            dc.save();

            dc.rectangle(clipRect);
            dc.clip();
            dc.translate(screenClientBounds.Left(), screenClientBounds.Top());

            OnDrawOver(dc);
            dc.restore();
        }
    }
}
void Lv2cElement::Mount(Lv2cWindow *window)
{
    this->window = window;
    if (this->window == nullptr)
    {
        throw std::invalid_argument("Invlid argument.");
    }
    OnMount(window);
}
void Lv2cElement::Unmount(Lv2cWindow *window)
{
    if (this->window != nullptr)
    {
        OnUnmount(window);
        if (Hascapture())
        {
            window->Capture(nullptr);
        }
        if (Focused())
        {
            window->Focus(nullptr);
        }
        this->window = nullptr;
    }
}

const Lv2cWindow *Lv2cElement::Window() const
{
    return window;
}
Lv2cWindow *Lv2cElement::Window()
{
    return window;
}

void Lv2cElement::Invalidate()
{
    if (!this->layoutValid)
        return;
    if (this->Style().Visibility() != Lv2cVisibility::Visible)
    {
        return;
    }
    InvalidateScreenRect(this->screenBounds);
}

void Lv2cElement::InvalidateScreenRect(const Lv2cRectangle &screenRect)
{
    if (layoutValid)
    {
        if (this->parentElement != nullptr)
        {
            parentElement->InvalidateScreenRect(screenRect);
        }
        else
        {
            if (this->window)
            {
                this->window->Invalidate(screenRect);
            }
        }
    }
}

void Lv2cElement::InvalidateClientRect(const Lv2cRectangle &clientRect)
{
    Lv2cRectangle screenRect = clientRect.translate(
        Lv2cPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top()));
    InvalidateScreenRect(screenRect);
}

void Lv2cElement::InvalidateLayout()
{
    layoutValid = false;
    if (parentElement)
    {
        parentElement->InvalidateLayout();
    }
    else
    {
        if (window)
        {
            window->InvalidateLayout();
        }
    }
}

void Lv2cElement::Layout(const Lv2cRectangle &layout)
{

    layoutValid = true;
    this->bounds = layout;
    this->borderBounds = removeThickness(this->bounds, Style().Margin());
    this->paddingBounds = removeThickness(this->borderBounds, Style().BorderWidth());
    this->clientBounds = removeThickness(this->paddingBounds, Style().Padding());
    this->clientSize = Lv2cSize(clientBounds.Width(), clientBounds.Height());
}

Lv2cSize Lv2cElement::removeThickness(Lv2cSize available, const Lv2cThicknessMeasurement &thickness)
{
    return Lv2cSize(
        std::max(available.Width() - thickness.Left().PixelValue() - thickness.Right().PixelValue(), 0.0),
        std::max(available.Height() - thickness.Top().PixelValue() - thickness.Bottom().PixelValue(), 0.0));
}
Lv2cSize Lv2cElement::AddThickness(Lv2cSize size, const Lv2cThicknessMeasurement &thickness)
{
    return Lv2cSize(
        size.Width() + thickness.Left().PixelValue() + thickness.Right().PixelValue(),
        size.Height() + thickness.Top().PixelValue() + thickness.Bottom().PixelValue());
}
Lv2cRectangle Lv2cElement::AddThickness(const Lv2cRectangle &rectangle, const Lv2cThicknessMeasurement &thickness)
{
    return Lv2cRectangle(
        rectangle.Left() - thickness.Left().PixelValue(),
        rectangle.Top() - thickness.Top().PixelValue(),
        rectangle.Width() + thickness.Left().PixelValue() + thickness.Right().PixelValue(),
        rectangle.Height() + thickness.Top().PixelValue() + thickness.Bottom().PixelValue());
}
Lv2cRectangle Lv2cElement::removeThickness(const Lv2cRectangle &rectangle, const Lv2cThicknessMeasurement &thickness)
{
    return Lv2cRectangle(
        rectangle.Left() + thickness.Left().PixelValue(),
        rectangle.Top() + thickness.Top().PixelValue(),
        rectangle.Width() - thickness.Left().PixelValue() - thickness.Right().PixelValue(),
        rectangle.Height() - thickness.Top().PixelValue() - thickness.Bottom().PixelValue());
}

void Lv2cElement::SetMeasure(Lv2cSize measuredSize)
{
    this->measure = measuredSize;
}
Lv2cSize Lv2cElement::MeasuredSize() const
{
    return this->measure;
}

bool Lv2cElement::WillDraw() const
{
    return (HasBackground() || WillDrawBorder());
}

bool Lv2cElement::WillDrawOver() const
{
    return false;
}

bool Lv2cElement::HasBackground() const
{
    return !this->Style().Background().isEmpty();
}
bool Lv2cElement::WillDrawBorder() const
{
    return !this->Style().BorderColor().isEmpty() && !this->Style().BorderWidth().isEmpty();
}

Lv2cSize Lv2cElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
{
    double width = Style().Width().PixelValue();
    double height = Style().Height().PixelValue();
    return Lv2cSize(width, height);
}

void Lv2cElement::Measure(Lv2cSize constraint, Lv2cSize available, Lv2cDrawingContext &context)
{
    Lv2cStyle &style = Style();

    if (Style().HorizontalAlignment() != Lv2cAlignment::Stretch)
    {
        constraint.Width(0);
    }
    else
    {
        // if (constraint.Width() == 0)
        // {
        //     LogWarning("Element has HorizontalAlignment()==Stretch, but parent doesn't have a fixed width.");
        //     LogWarning("Add HorizontalAlignment()==Stretch to the parent container, or set a Width().");
        // }
    }
    if (Style().VerticalAlignment() != Lv2cAlignment::Stretch)
    {
        constraint.Height(0);
    }
    else
    {
        // if (constraint.Height() == 0)
        // {
        //     LogWarning("Element has VerticalAlignment()==Stretch, but parent doesn't have a fixed height.");
        //     LogWarning("Add VerticalAlignment()==Stretch to the parent container, or set a Height().");
        // }
    }
    style.SetStyleContext(available); // % in margins(!) urgh.
    Lv2cSize borderAvailable = removeThickness(available, style.Margin());
    Lv2cSize measureConstraint = removeThickness(constraint, style.Margin());
    style.SetStyleContext(borderAvailable); // establishes % going forward.

    this->roundCorners = style.RoundCorners().PixelValue(); // resolve round corner percentages.

    Lv2cSize borderConstraint = this->MeasuredSizeSizeFromStyle(measureConstraint);
    Lv2cSize paddingConstraint = removeThickness(borderConstraint, Style().BorderWidth());
    Lv2cSize clientConstraint = removeThickness(paddingConstraint, Style().Padding());

    if (borderConstraint.Height() != 0 && borderAvailable.Height() > borderConstraint.Height())
    {
        borderAvailable.Height(borderConstraint.Height());
    }
    if (borderConstraint.Width() != 0 && borderAvailable.Width() > borderConstraint.Width())
    {
        borderAvailable.Width(borderConstraint.Width());
    }

    Lv2cSize paddingAvailable = removeThickness(borderAvailable, style.BorderWidth());
    Lv2cSize clientAvailable = removeThickness(paddingAvailable, style.Padding());

    auto maxWidth = Style().MaxWidth();
    double maxWidthPixels = std::numeric_limits<double>::max();
    if (maxWidth.has_value())
    {
        maxWidthPixels = maxWidth.value().PixelValue();
        if (clientAvailable.Width() != 0 && maxWidthPixels < clientAvailable.Width())
        {
            clientAvailable.Width(maxWidthPixels);
        }
        if (clientConstraint.Width() != 0 || clientConstraint.Width() > maxWidthPixels)
        {
            clientConstraint.Width(maxWidthPixels);
        }
    }

    auto minWidth = Style().MinWidth();
    double minWidthPixels = 0;
    if (minWidth.has_value())
    {
        minWidthPixels = minWidth.value().PixelValue();
        if (clientAvailable.Width() != 0 && clientAvailable.Width() < minWidthPixels)
        {
            clientAvailable.Width(minWidthPixels);
        }
        if (clientConstraint.Width() != 0 && clientConstraint.Width() < minWidthPixels)
        {
            clientConstraint.Width(minWidthPixels);
        }
        if (clientConstraint.Width() != 0 || clientConstraint.Width() > maxWidthPixels)
        {
            clientConstraint.Width(maxWidthPixels);
        }
    }

    auto result = MeasureClient(clientConstraint, clientAvailable, context);

    if (result.Width() > maxWidthPixels)
    {
        result.Width(maxWidthPixels);
    }
    if (result.Width() < minWidthPixels)
    {
        result.Width(minWidthPixels);
    }
    result = AddThickness(result, Style().Padding());
    result = AddThickness(result, Style().BorderWidth());

    result = Lv2cSize(
        std::max(result.Width(), 0.0),
        std::max(result.Height(), 0.0));
    result = AddThickness(result, Style().Margin());
    if (Style().HorizontalAlignment() == Lv2cAlignment::Stretch && constraint.Width() > result.Width())
    {
        result.Width(constraint.Width());
    }
    if (Style().VerticalAlignment() == Lv2cAlignment::Stretch && constraint.Height() > result.Height())
    {
        result.Height(constraint.Height());
    }
    if (result.Height() > available.Height())
    {
        result.Height(available.Height());
    }
    if (result.Width() > available.Width())
    {
        result.Width(available.Width());
    }

    SetMeasure(result);
}

Lv2cSize Lv2cElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    return available;
}

Lv2cRectangle Lv2cElement::GetDrawBounds(
    const Lv2cRectangle &screenBounds,
    const Lv2cRectangle &borderBounds)
{
    return screenBounds;
}

void Lv2cElement::FinalizeLayout(const Lv2cRectangle &layoutClipRect, const Lv2cRectangle &parentBounds, bool clippedInLayout)
{
    this->layoutValid = true;
    this->savedLayoutClipRect = layoutClipRect; // saved in case we want to re-do layout.
    this->savedClippedInLayout = clippedInLayout;
    Lv2cPoint offset = Lv2cPoint(parentBounds.Left(), parentBounds.Top());

    Lv2cRectangle oldBounds = this->screenDrawBounds;
    this->clippedInLayout = clippedInLayout;
    if (clippedInLayout)
    {
        this->screenBounds =
            this->screenBorderBounds =
                this->screenPaddingBounds =
                    this->screenClientBounds =
                        this->screenDrawBounds = Lv2cRectangle(-100, -100, 0, 0);
    }
    else
    {
        this->screenBounds = this->bounds.translate(offset);
        this->screenBorderBounds = this->borderBounds.translate(offset);
        this->screenPaddingBounds = this->paddingBounds.translate(offset);
        this->screenClientBounds = this->clientBounds.translate(offset);
        this->screenDrawBounds = GetDrawBounds(screenBounds, screenBorderBounds);
        if (!layoutClipRect.Intersects(this->screenBounds))
        {
            this->clippedInLayout = true;
        }
    }

    if (oldBounds != this->screenDrawBounds)
    {
        InvalidateScreenRect(oldBounds);
        InvalidateScreenRect(this->screenDrawBounds);
    }
    this->layoutValid = true;
}

bool Lv2cElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    if (MouseDown.Fire(event))
    {
        return true;
    }
    return false;
}
bool Lv2cElement::OnScrollWheel(Lv2cScrollWheelEventArgs &event)
{
    if (ScrollWheel.Fire(event))
    {
        return true;
    }
    return false;
}


bool Lv2cElement::OnMouseUp(Lv2cMouseEventArgs &event)
{
    if (MouseUp.Fire(event))
    {
        return true;
    }
    return false;
}
bool Lv2cElement::OnMouseMove(Lv2cMouseEventArgs &event)
{
    if (MouseMove.Fire(event))
    {
        return true;
    }
    return false;
}

bool Lv2cElement::OnMouseOver(Lv2cMouseOverEventArgs &event)
{
    if (MouseOver.Fire(event))
    {
        return true;
    }
    return false;
}

bool Lv2cElement::OnMouseOut(Lv2cMouseOverEventArgs &event)
{
    if (MouseOut.Fire(event))
    {
        return true;
    }
    return false;
}

void Lv2cElement::SetMouseOver(bool mouseOver)
{
    if (mouseOver != this->mouseOver)
    {
        this->mouseOver = mouseOver;
        Lv2cMouseOverEventArgs e{mouseOver};
        if (this->mouseOver)
        {
            HoverState(HoverState() + Lv2cHoverState::Hover);
            OnMouseOver(e);
        }
        else
        {
            OnMouseOut(e);
            HoverState(HoverState() - Lv2cHoverState::Hover);
        }
    }
}

bool Lv2cElement::OnKeycodeDown(const Lv2cKeyboardEventArgs &event)
{
    return false;
}
bool Lv2cElement::OnKeycodeUp(const Lv2cKeyboardEventArgs &event)
{
    return false;
}

bool Lv2cElement::FireKeyDown(const Lv2cKeyboardEventArgs &event)
{
    Lv2cElement *element = this;

    while (element != nullptr)
    {
        if (element->OnKeyDown(event))
        {
            return true;
        }
        element = element->Parent();
    }
    return false;
}

bool Lv2cElement::FireScrollWheel(Lv2cScrollWheelEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
    {
        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            event.point = event.screenPoint - Lv2cPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top());
            if (OnScrollWheel(event))
            {
                return true;
            }
        }
    }
    return false;
}


bool Lv2cElement::FireMouseDown(Lv2cMouseEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
    {
        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            event.point = event.screenPoint - Lv2cPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top());
            if (OnMouseDown(event))
            {
                return true;
            }
        }
    }
    return false;
}

bool Lv2cElement::FireMouseUp(Lv2cMouseEventArgs &event)
{
    if (Style().Visibility() == Lv2cVisibility::Visible)
    {
        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            event.point = event.screenPoint - Lv2cPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top());
            if (OnMouseUp(event))
            {
                return true;
            }
        }
    }
    return false;
}

bool Lv2cElement::CaptureMouse()
{
    if (window)
    {
        return window->Capture(this);
    }
    else
    {
        return false;
    }
}

bool Lv2cElement::Hascapture() const
{
    return Capture() == this;
}
const Lv2cElement *Lv2cElement::Capture() const
{
    if (window)
    {
        return window->Capture();
    }
    return nullptr;
}
void Lv2cElement::ReleaseCapture()
{
    window->releaseCapture(this);
}

void Lv2cElement::UpdateMouseOver(Lv2cPoint mousePosition)
{
    if (clippedInLayout)
        return;

    if (window && window->Capture())
    {
        SetMouseOver(this == window->Capture() && this->screenBorderBounds.Contains(mousePosition));
    }
    else
    {
        bool visible = (Style().Visibility() == Lv2cVisibility::Visible);

        SetMouseOver(visible && this->screenBorderBounds.Contains(mousePosition));
    }
}

Lv2cElement &Lv2cElement::ClearClasses()
{
    if (classes.size() != 0)
    {
        classes.resize(0);
        InvalidateLayout();
    }
    return *this;
}
Lv2cElement &Lv2cElement::AddClass(Lv2cStyle::ptr style)
{
    if (!style)
        return *this;
    classes.insert(classes.begin(), style);
    return *this;
}
Lv2cElement &Lv2cElement::RemoveClass(Lv2cStyle::ptr style)
{
    if (!style)
        return *this;
    for (auto i = classes.begin(); i != classes.end(); ++i)
    {
        if ((*i).get() == style.get())
        {
            classes.erase(i);
            break;
        }
    }
    return *this;
}

const std::vector<Lv2cStyle::ptr> &Lv2cElement::Classes() const
{
    return classes;
}

bool Lv2cElement::IsMounted() const
{
    return this->window != nullptr;
}

Lv2cElement &Lv2cElement::Theme(Lv2cTheme::ptr theme)
{
    this->theme = theme;
    Invalidate();
    InvalidateLayout();
    return *this;
}

const Lv2cTheme &Lv2cElement::Theme() const
{
    if (this->theme)
    {
        return *(theme.get());
    }
    if (parentElement != nullptr)
    {
        return parentElement->Theme();
    }
    if (this->window == nullptr)
    {
        throw std::runtime_error("Can't call this method if not mounted.");
    }
    return this->window->Theme();
}

Lv2cElement &Lv2cElement::Classes(Lv2cStyle::ptr style)
{
    this->classes.resize(0);
    if (style)
    {
        this->classes.push_back(style);
    }
    return *this;
}
Lv2cElement &Lv2cElement::Classes(std::vector<Lv2cStyle::ptr> styles)
{
    this->classes.resize(0);
    this->classes.reserve(styles.size());
    for (auto &style : styles)
    {
        if (style)
        {
            this->classes.push_back(style);
        }
    }
    return *this;
}

Lv2cSize Lv2cElement::MeasuredSizeSizeFromStyle(Lv2cSize available)
{

    Lv2cStyle &style = Style();
    Lv2cSize size{0, 0};
    {
        auto width = Style().Width();
        if (width.isEmpty() || width.PixelValue() == 0)
        {
            auto alignment = style.HorizontalAlignment();
            if (alignment == Lv2cAlignment::Stretch)
            {
                size.Width(available.Width());
            }
            else
            {
                size.Width(0);
            }
        }
        else if (width.isPercent())
        {
            size.Width(available.Width() * width.getBaseValue() / 100);
        }
        else
        {
            size.Width(width.PixelValue());
        }
    }

    {
        auto height = style.Height();
        if (height.isEmpty() || height.PixelValue() == 0)
        {
            auto alignment = style.VerticalAlignment();
            if (alignment == Lv2cAlignment::Stretch)
            {
                size.Height(available.Height());
            }
            else
            {
                size.Height(0);
            }
        }
        else if (height.isPercent())
        {
            size.Height(available.Height() * height.getBaseValue() / 100);
        }
        else
        {
            size.Height(height.PixelValue());
        }
    }
    return size;
}

PangoContext *Lv2cElement::GetPangoContext()
{
    if (this->window == nullptr)
    {
        throw std::runtime_error("Not mounted.");
    }
    return this->window->GetPangoContext();
}

void Lv2cElement::OnDrawOver(Lv2cDrawingContext &dc)
{
}

static std::ostream &operator<<(std::ostream &os, const Lv2cRectangle &rect)
{
    os << rect.Left() << "," << rect.Top() << "," << rect.Width() << "," << rect.Height();
    return os;
}
void Lv2cElement::PrintStructure(std::ostream &cout, size_t indent) const
{
    for (size_t i = 0; i < indent; ++i)
    {
        cout << " ";
    }
    cout << this->Tag()
         << " bounds='" << this->screenBorderBounds << "'" << std::endl;

    if (isContainer())
    {
        Lv2cContainerElement *container = (Lv2cContainerElement *)this;
        for (auto &child : container->LayoutChildren())
        {
            child->PrintStructure(cout, indent + 4);
        }
    }
}

void Lv2cElement::PrintStructure(std::ostream &cout) const
{
    PrintStructure(cout, 0);
}

void Lv2cElement::PrintStructure() const
{
    PrintStructure(std::cout);
}

bool Lv2cElement::OnFocus(const Lv2cFocusEventArgs &eventArgs)
{
    HoverState(HoverState() + Lv2cHoverState::Focus);
    return FocusEvent.Fire(eventArgs);
}
bool Lv2cElement::OnLostFocus(const Lv2cFocusEventArgs &eventArgs)
{
    HoverState(HoverState() - Lv2cHoverState::Focus);

    return LostFocusEvent.Fire(eventArgs);
}

bool Lv2cElement::Focused() const
{
    if (Window() != nullptr)
    {
        return this->window->FocusedElement() == this;
    }
    return false;
}

bool Lv2cElement::Focus()
{
    if (Window() != nullptr)
    {
        return Window()->Focus(this);
    }
    return false;
}
bool Lv2cElement::ReleaseFocus()
{
    if (Window() != nullptr)
    {
        if (Window()->FocusedElement() == this)
        {
            Window()->releaseFocus(this);
            return true;
        }
    }
    return false;
}

bool Lv2cElement::Focus(const Lv2cFocusEventArgs &eventArgs)
{
    return OnFocus(eventArgs);
}
bool Lv2cElement::LostFocus(const Lv2cFocusEventArgs &eventArgs)
{
    return OnLostFocus(eventArgs);
}

bool Lv2cElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
{
    if (KeyDown.Fire(event))
    {
        return true;
    }
    return false;
}

bool Lv2cElement::WantsFocus() const
{
    return false;
}

const Lv2cRectangle &Lv2cElement::ScreenBounds() const
{
    return this->screenBounds;
}
const Lv2cRectangle &Lv2cElement::ScreenBorderRect() const
{
    return this->screenBorderBounds;
}
const Lv2cRectangle &Lv2cElement::ScreenClientBounds() const
{
    return this->screenClientBounds;
}

void Lv2cElement::OnHoverStateChanged(Lv2cHoverState hoverState)
{
}

Lv2cHoverState Lv2cElement::HoverState() const
{
    return hoverState;
}
Lv2cElement &Lv2cElement::HoverState(Lv2cHoverState hoverState)
{
    if (this->hoverState != hoverState)
    {
        this->hoverState = hoverState;
        OnHoverStateChanged(hoverState);
    }
    return *this;
}

Lv2cElement &Lv2cElement::UserData(const Lv2cUserData::ptr &value)
{
    this->userData = value;
    return *this;
}
Lv2cElement &Lv2cElement::UserData(Lv2cUserData::ptr &&value)
{
    this->userData = std::move(value);
    return *this;
}
Lv2cUserData::ptr Lv2cElement::UserData()
{
    return this->userData;
}

bool Lv2cElement::ClipChildren() const
{
    return false;
}

void Lv2cElement::InvalidateParentLayout()
{
    if (window && layoutValid)
    {
        if (parentElement)
        {
            parentElement->PartialLayout();
        }
        else
        {
            PartialLayout();
        }
    }
}

void Lv2cElement::PartialLayout()
{
    if (!window)
        return;
    if (!layoutValid)
    {
        return;
    }
    Lv2cDrawingContext context = window->CreateDrawingContext();
    Lv2cSize size{this->clientBounds.Width(), this->clientBounds.Height()};
    Measure(size, size, context);
    Arrange(size, context);
    FinalizeLayout(this->savedLayoutClipRect, this->Parent()->screenClientBounds, this->savedClippedInLayout);
    Invalidate();
}

Lv2cRectangle Lv2cElement::ClientBorderRectangle() const
{
    if (screenBorderBounds.Empty())
        return Lv2cRectangle();
    return screenBorderBounds.translate(Lv2cPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}
Lv2cRectangle Lv2cElement::ClientPaddingRectangle() const
{
    if (screenPaddingBounds.Empty())
        return Lv2cRectangle();
    return screenPaddingBounds.translate(Lv2cPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}
Lv2cRectangle Lv2cElement::ClientMarginRectangle() const
{
    if (screenBounds.Empty())
        return Lv2cRectangle();
    return screenBounds.translate(Lv2cPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}

void Lv2cElement::OnLayoutComplete()
{
    
}

bool Lv2cElement::LayoutValid() const {
    return this->layoutValid;
}

