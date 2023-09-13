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

#include "lvtk/LvtkElement.hpp"
#include "lvtk/LvtkLog.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkTypes.hpp"
#include "lvtk/LvtkContainerElement.hpp"
#include <stdexcept>
#include <iostream>
#include <numbers>
#include "cleanup.hpp"
#include "ss.hpp"

using namespace lvtk;

LvtkElement::LvtkElement()
{
    this->Style().SetElement(this);
}

LvtkElement::~LvtkElement() noexcept
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

static void drawRoundRect(LvtkDrawingContext &dc, const LvtkRectangle &bounds, LvtkRoundCorners corners)
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

void LvtkElement::drawRoundBorderRect(LvtkDrawingContext &dc)
{
    LvtkRectangle borderBounds = this->borderBounds.Translate(
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
void LvtkElement::drawRoundInsideBorderRect(LvtkDrawingContext &dc)
{
    LvtkRectangle borderInnerBounds = this->paddingBounds.Translate(
        -this->clientBounds.Left(),
        -this->clientBounds.Top());

    LvtkRoundCorners corners = this->roundCorners;
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

void LvtkElement::OnDraw(LvtkDrawingContext &dc)
{

    bool hasRoundCorners = !this->Style().RoundCorners().isEmpty();

    if (hasRoundCorners)
    {
        dc.save();
        drawRoundBorderRect(dc);
        dc.clip();

        if (!this->Style().Background().isEmpty())
        {
            LvtkRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            dc.set_source(this->Style().Background());
            dc.rectangle(borderBounds);
            dc.fill();
        }
        if (this->WillDrawBorder())
        {

            LvtkRectangle borderBounds = this->borderBounds.Translate(
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
            LvtkRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            dc.set_source(this->Style().Background());
            dc.rectangle(borderBounds);
            dc.fill();
        }
        if (this->WillDrawBorder())
        {
            LvtkRectangle borderBounds = this->borderBounds.Translate(
                -this->clientBounds.Left(),
                -this->clientBounds.Top());
            LvtkRectangle borderInnerBounds = this->paddingBounds.Translate(
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

void LvtkElement::OnMount()
{
}

void LvtkElement::OnUnmount()
{
}
void LvtkElement::OnMount(LvtkWindow *window)
{
    OnMount();
    OnMounted.Fire(window);
}
void LvtkElement::OnUnmount(LvtkWindow *window)
{
    OnUnmounted.Fire(window);
    OnUnmount();


}

void LvtkElement::Draw(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds)
{
    if (clippedInLayout)
    {

        return;
    }

    dc.check_status();

    if (Style().Visibility() != LvtkVisibility::Visible)
    {
        return;
    }
    if (Style().Opacity() != 1.0)
    {
        double windowScale = Window()->WindowScale();

        LvtkRectangle bounds = clipBounds.Intersect(this->screenDrawBounds);
        if (bounds.Empty())
            return;
        LvtkRectangle deviceBounds = dc.user_to_device(bounds);
        double left = std::floor(deviceBounds.Left());
        double top = std::floor(deviceBounds.Top());
        double right = std::ceil(deviceBounds.Right());
        double bottom = std::ceil(deviceBounds.Bottom());
        deviceBounds = LvtkRectangle(left, top, right - left, bottom - top);

        LvtkRectangle screenBounds = dc.device_to_user(deviceBounds);

        cairo_public cairo_surface_t *renderSurface = cairo_image_surface_create(
            cairo_format_t::CAIRO_FORMAT_ARGB32,
            (int)std::round(deviceBounds.Width()),
            (int)std::round(deviceBounds.Height()));
        {
            LvtkDrawingContext bdc(renderSurface);

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
            LogError(SS("Drawing error: " << LvtkStatusMessage(dc.status())));
        }
    }
}
void LvtkElement::DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds)
{
    if (!clipBounds.Intersects(this->screenDrawBounds))
        return;

    if (this->Style().Visibility() != LvtkVisibility::Visible)
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
        LvtkRectangle clipRect = screenBorderBounds.Intersect(clipBounds);
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
        LvtkRectangle clipRect = screenBorderBounds.Intersect(clipBounds);
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
void LvtkElement::Mount(LvtkWindow *window)
{
    this->window = window;
    if (this->window == nullptr)
    {
        throw std::invalid_argument("Invlid argument.");
    }
    OnMount(window);
}
void LvtkElement::Unmount(LvtkWindow *window)
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

const LvtkWindow *LvtkElement::Window() const
{
    return window;
}
LvtkWindow *LvtkElement::Window()
{
    return window;
}

void LvtkElement::Invalidate()
{
    if (!this->layoutValid)
        return;
    if (this->Style().Visibility() != LvtkVisibility::Visible)
    {
        return;
    }
    InvalidateScreenRect(this->screenBounds);
}

void LvtkElement::InvalidateScreenRect(const LvtkRectangle &screenRect)
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

void LvtkElement::InvalidateClientRect(const LvtkRectangle &clientRect)
{
    LvtkRectangle screenRect = clientRect.translate(
        LvtkPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top()));
    InvalidateScreenRect(screenRect);
}

void LvtkElement::InvalidateLayout()
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

void LvtkElement::Layout(const LvtkRectangle &layout)
{

    layoutValid = true;
    this->bounds = layout;
    this->borderBounds = removeThickness(this->bounds, Style().Margin());
    this->paddingBounds = removeThickness(this->borderBounds, Style().BorderWidth());
    this->clientBounds = removeThickness(this->paddingBounds, Style().Padding());
    this->clientSize = LvtkSize(clientBounds.Width(), clientBounds.Height());
}

LvtkSize LvtkElement::removeThickness(LvtkSize available, const LvtkThicknessMeasurement &thickness)
{
    return LvtkSize(
        std::max(available.Width() - thickness.Left().PixelValue() - thickness.Right().PixelValue(), 0.0),
        std::max(available.Height() - thickness.Top().PixelValue() - thickness.Bottom().PixelValue(), 0.0));
}
LvtkSize LvtkElement::AddThickness(LvtkSize size, const LvtkThicknessMeasurement &thickness)
{
    return LvtkSize(
        size.Width() + thickness.Left().PixelValue() + thickness.Right().PixelValue(),
        size.Height() + thickness.Top().PixelValue() + thickness.Bottom().PixelValue());
}
LvtkRectangle LvtkElement::AddThickness(const LvtkRectangle &rectangle, const LvtkThicknessMeasurement &thickness)
{
    return LvtkRectangle(
        rectangle.Left() - thickness.Left().PixelValue(),
        rectangle.Top() - thickness.Top().PixelValue(),
        rectangle.Width() + thickness.Left().PixelValue() + thickness.Right().PixelValue(),
        rectangle.Height() + thickness.Top().PixelValue() + thickness.Bottom().PixelValue());
}
LvtkRectangle LvtkElement::removeThickness(const LvtkRectangle &rectangle, const LvtkThicknessMeasurement &thickness)
{
    return LvtkRectangle(
        rectangle.Left() + thickness.Left().PixelValue(),
        rectangle.Top() + thickness.Top().PixelValue(),
        rectangle.Width() - thickness.Left().PixelValue() - thickness.Right().PixelValue(),
        rectangle.Height() - thickness.Top().PixelValue() - thickness.Bottom().PixelValue());
}

void LvtkElement::SetMeasure(LvtkSize measuredSize)
{
    this->measure = measuredSize;
}
LvtkSize LvtkElement::MeasuredSize() const
{
    return this->measure;
}

bool LvtkElement::WillDraw() const
{
    return (HasBackground() || WillDrawBorder());
}

bool LvtkElement::WillDrawOver() const
{
    return false;
}

bool LvtkElement::HasBackground() const
{
    return !this->Style().Background().isEmpty();
}
bool LvtkElement::WillDrawBorder() const
{
    return !this->Style().BorderColor().isEmpty() && !this->Style().BorderWidth().isEmpty();
}

LvtkSize LvtkElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context)
{
    double width = Style().Width().PixelValue();
    double height = Style().Height().PixelValue();
    return LvtkSize(width, height);
}

void LvtkElement::Measure(LvtkSize constraint, LvtkSize available, LvtkDrawingContext &context)
{
    LvtkStyle &style = Style();

    if (Style().HorizontalAlignment() != LvtkAlignment::Stretch)
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
    if (Style().VerticalAlignment() != LvtkAlignment::Stretch)
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
    LvtkSize borderAvailable = removeThickness(available, style.Margin());
    LvtkSize measureConstraint = removeThickness(constraint, style.Margin());
    style.SetStyleContext(borderAvailable); // establishes % going forward.

    this->roundCorners = style.RoundCorners().PixelValue(); // resolve round corner percentages.

    LvtkSize borderConstraint = this->MeasuredSizeSizeFromStyle(measureConstraint);
    LvtkSize paddingConstraint = removeThickness(borderConstraint, Style().BorderWidth());
    LvtkSize clientConstraint = removeThickness(paddingConstraint, Style().Padding());

    if (borderConstraint.Height() != 0 && borderAvailable.Height() > borderConstraint.Height())
    {
        borderAvailable.Height(borderConstraint.Height());
    }
    if (borderConstraint.Width() != 0 && borderAvailable.Width() > borderConstraint.Width())
    {
        borderAvailable.Width(borderConstraint.Width());
    }

    LvtkSize paddingAvailable = removeThickness(borderAvailable, style.BorderWidth());
    LvtkSize clientAvailable = removeThickness(paddingAvailable, style.Padding());

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

    result = LvtkSize(
        std::max(result.Width(), 0.0),
        std::max(result.Height(), 0.0));
    result = AddThickness(result, Style().Margin());
    if (Style().HorizontalAlignment() == LvtkAlignment::Stretch && constraint.Width() > result.Width())
    {
        result.Width(constraint.Width());
    }
    if (Style().VerticalAlignment() == LvtkAlignment::Stretch && constraint.Height() > result.Height())
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

LvtkSize LvtkElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    return available;
}

LvtkRectangle LvtkElement::GetDrawBounds(
    const LvtkRectangle &screenBounds,
    const LvtkRectangle &borderBounds)
{
    return screenBounds;
}

void LvtkElement::FinalizeLayout(const LvtkRectangle &layoutClipRect, const LvtkRectangle &parentBounds, bool clippedInLayout)
{
    this->layoutValid = true;
    this->savedLayoutClipRect = layoutClipRect; // saved in case we want to re-do layout.
    this->savedClippedInLayout = clippedInLayout;
    LvtkPoint offset = LvtkPoint(parentBounds.Left(), parentBounds.Top());

    LvtkRectangle oldBounds = this->screenDrawBounds;
    this->clippedInLayout = clippedInLayout;
    if (clippedInLayout)
    {
        this->screenBounds =
            this->screenBorderBounds =
                this->screenPaddingBounds =
                    this->screenClientBounds =
                        this->screenDrawBounds = LvtkRectangle(-100, -100, 0, 0);
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

bool LvtkElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    if (MouseDown.Fire(event))
    {
        return true;
    }
    return false;
}
bool LvtkElement::OnMouseUp(LvtkMouseEventArgs &event)
{
    if (MouseUp.Fire(event))
    {
        return true;
    }
    return false;
}
bool LvtkElement::OnMouseMove(LvtkMouseEventArgs &event)
{
    if (MouseMove.Fire(event))
    {
        return true;
    }
    return false;
}

bool LvtkElement::OnMouseOver(LvtkMouseOverEventArgs &event)
{
    if (MouseOver.Fire(event))
    {
        return true;
    }
    return false;
}

bool LvtkElement::OnMouseOut(LvtkMouseOverEventArgs &event)
{
    if (MouseOut.Fire(event))
    {
        return true;
    }
    return false;
}

void LvtkElement::SetMouseOver(bool mouseOver)
{
    if (mouseOver != this->mouseOver)
    {
        this->mouseOver = mouseOver;
        LvtkMouseOverEventArgs e{mouseOver};
        if (this->mouseOver)
        {
            HoverState(HoverState() + LvtkHoverState::Hover);
            OnMouseOver(e);
        }
        else
        {
            OnMouseOut(e);
            HoverState(HoverState() - LvtkHoverState::Hover);
        }
    }
}

bool LvtkElement::OnKeycodeDown(const LvtkKeyboardEventArgs &event)
{
    return false;
}
bool LvtkElement::OnKeycodeUp(const LvtkKeyboardEventArgs &event)
{
    return false;
}

bool LvtkElement::FireKeyDown(const LvtkKeyboardEventArgs &event)
{
    LvtkElement *element = this;

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

bool LvtkElement::FireMouseDown(LvtkMouseEventArgs &event)
{
    if (Style().Visibility() == LvtkVisibility::Visible)
    {
        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            event.point = event.screenPoint - LvtkPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top());
            if (OnMouseDown(event))
            {
                return true;
            }
        }
    }
    return false;
}

bool LvtkElement::FireMouseUp(LvtkMouseEventArgs &event)
{
    if (Style().Visibility() == LvtkVisibility::Visible)
    {
        if (this->screenBorderBounds.Contains(event.screenPoint))
        {
            event.point = event.screenPoint - LvtkPoint(this->screenClientBounds.Left(), this->screenClientBounds.Top());
            if (OnMouseUp(event))
            {
                return true;
            }
        }
    }
    return false;
}

bool LvtkElement::CaptureMouse()
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

bool LvtkElement::Hascapture() const
{
    return Capture() == this;
}
const LvtkElement *LvtkElement::Capture() const
{
    if (window)
    {
        return window->Capture();
    }
    return nullptr;
}
void LvtkElement::ReleaseCapture()
{
    window->releaseCapture(this);
}

void LvtkElement::UpdateMouseOver(LvtkPoint mousePosition)
{
    if (clippedInLayout)
        return;

    if (window && window->Capture())
    {
        SetMouseOver(this == window->Capture() && this->screenBorderBounds.Contains(mousePosition));
    }
    else
    {
        bool visible = (Style().Visibility() == LvtkVisibility::Visible);

        SetMouseOver(visible && this->screenBorderBounds.Contains(mousePosition));
    }
}

LvtkElement &LvtkElement::ClearClasses()
{
    if (classes.size() != 0)
    {
        classes.resize(0);
        InvalidateLayout();
    }
    return *this;
}
LvtkElement &LvtkElement::AddClass(LvtkStyle::ptr style)
{
    if (!style)
        return *this;
    classes.insert(classes.begin(), style);
    return *this;
}
LvtkElement &LvtkElement::RemoveClass(LvtkStyle::ptr style)
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

const std::vector<LvtkStyle::ptr> &LvtkElement::Classes() const
{
    return classes;
}

bool LvtkElement::IsMounted() const
{
    return this->window != nullptr;
}

LvtkElement &LvtkElement::Theme(LvtkTheme::ptr theme)
{
    this->theme = theme;
    Invalidate();
    InvalidateLayout();
    return *this;
}

const LvtkTheme &LvtkElement::Theme() const
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

LvtkElement &LvtkElement::Classes(LvtkStyle::ptr style)
{
    this->classes.resize(0);
    if (style)
    {
        this->classes.push_back(style);
    }
    return *this;
}
LvtkElement &LvtkElement::Classes(std::vector<LvtkStyle::ptr> styles)
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

LvtkSize LvtkElement::MeasuredSizeSizeFromStyle(LvtkSize available)
{

    LvtkStyle &style = Style();
    LvtkSize size{0, 0};
    {
        auto width = Style().Width();
        if (width.isEmpty() || width.PixelValue() == 0)
        {
            auto alignment = style.HorizontalAlignment();
            if (alignment == LvtkAlignment::Stretch)
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
            if (alignment == LvtkAlignment::Stretch)
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

PangoContext *LvtkElement::GetPangoContext()
{
    if (this->window == nullptr)
    {
        throw std::runtime_error("Not mounted.");
    }
    return this->window->GetPangoContext();
}

void LvtkElement::OnDrawOver(LvtkDrawingContext &dc)
{
}

static std::ostream &operator<<(std::ostream &os, const LvtkRectangle &rect)
{
    os << rect.Left() << "," << rect.Top() << "," << rect.Width() << "," << rect.Height();
    return os;
}
void LvtkElement::PrintStructure(std::ostream &cout, size_t indent) const
{
    for (size_t i = 0; i < indent; ++i)
    {
        cout << " ";
    }
    cout << this->Tag()
         << " bounds='" << this->screenBorderBounds << "'" << std::endl;

    if (isContainer())
    {
        LvtkContainerElement *container = (LvtkContainerElement *)this;
        for (auto &child : container->LayoutChildren())
        {
            child->PrintStructure(cout, indent + 4);
        }
    }
}

void LvtkElement::PrintStructure(std::ostream &cout) const
{
    PrintStructure(cout, 0);
}

void LvtkElement::PrintStructure() const
{
    PrintStructure(std::cout);
}

bool LvtkElement::OnFocus(const LvtkFocusEventArgs &eventArgs)
{
    HoverState(HoverState() + LvtkHoverState::Focus);
    return FocusEvent.Fire(eventArgs);
}
bool LvtkElement::OnLostFocus(const LvtkFocusEventArgs &eventArgs)
{
    HoverState(HoverState() - LvtkHoverState::Focus);

    return LostFocusEvent.Fire(eventArgs);
}

bool LvtkElement::Focused() const
{
    if (Window() != nullptr)
    {
        return this->window->FocusedElement() == this;
    }
    return false;
}

bool LvtkElement::Focus()
{
    if (Window() != nullptr)
    {
        return Window()->Focus(this);
    }
    return false;
}
bool LvtkElement::ReleaseFocus()
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

bool LvtkElement::Focus(const LvtkFocusEventArgs &eventArgs)
{
    return OnFocus(eventArgs);
}
bool LvtkElement::LostFocus(const LvtkFocusEventArgs &eventArgs)
{
    return OnLostFocus(eventArgs);
}

bool LvtkElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (KeyDown.Fire(event))
    {
        return true;
    }
    return false;
}

bool LvtkElement::WantsFocus() const
{
    return false;
}

const LvtkRectangle &LvtkElement::ScreenBounds() const
{
    return this->screenBounds;
}
const LvtkRectangle &LvtkElement::ScreenBorderRect() const
{
    return this->screenBorderBounds;
}
const LvtkRectangle &LvtkElement::ScreenClientBounds() const
{
    return this->screenClientBounds;
}

void LvtkElement::OnHoverStateChanged(LvtkHoverState hoverState)
{
}

LvtkHoverState LvtkElement::HoverState() const
{
    return hoverState;
}
LvtkElement &LvtkElement::HoverState(LvtkHoverState hoverState)
{
    if (this->hoverState != hoverState)
    {
        this->hoverState = hoverState;
        OnHoverStateChanged(hoverState);
    }
    return *this;
}

LvtkElement &LvtkElement::UserData(const LvtkUserData::ptr &value)
{
    this->userData = value;
    return *this;
}
LvtkElement &LvtkElement::UserData(LvtkUserData::ptr &&value)
{
    this->userData = std::move(value);
    return *this;
}
LvtkUserData::ptr LvtkElement::UserData()
{
    return this->userData;
}

bool LvtkElement::ClipChildren() const
{
    return false;
}

void LvtkElement::InvalidateParentLayout()
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

void LvtkElement::PartialLayout()
{
    if (!window)
        return;
    if (!layoutValid)
    {
        return;
    }
    LvtkDrawingContext context = window->CreateDrawingContext();
    LvtkSize size{this->clientBounds.Width(), this->clientBounds.Height()};
    Measure(size, size, context);
    Arrange(size, context);
    FinalizeLayout(this->savedLayoutClipRect, this->Parent()->screenClientBounds, this->savedClippedInLayout);
    Invalidate();
}

LvtkRectangle LvtkElement::ClientBorderRectangle() const
{
    if (screenBorderBounds.Empty())
        return LvtkRectangle();
    return screenBorderBounds.translate(LvtkPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}
LvtkRectangle LvtkElement::ClientPaddingRectangle() const
{
    if (screenPaddingBounds.Empty())
        return LvtkRectangle();
    return screenPaddingBounds.translate(LvtkPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}
LvtkRectangle LvtkElement::ClientMarginRectangle() const
{
    if (screenBounds.Empty())
        return LvtkRectangle();
    return screenBounds.translate(LvtkPoint(-screenClientBounds.Left(), -screenClientBounds.Top()));
}

void LvtkElement::OnLayoutComplete()
{
    
}

bool LvtkElement::LayoutValid() const {
    return this->layoutValid;
}