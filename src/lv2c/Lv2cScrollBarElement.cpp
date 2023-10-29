#include "lv2c/Lv2cScrollBarElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include <numbers>

using namespace lvtk;

Lv2cScrollBarElement::Lv2cScrollBarElement()
{
    OrientationProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    WindowSizeProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    DocumentSizeProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    ScrollOffsetProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    ThumbHoverColorsProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    TrackHoverColorsProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    AnimationValueProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    FlubberFactorProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);

}

bool Lv2cScrollBarElement::WillDraw() const
{
    return true;
}

static Lv2cRectangle BlendRect(double value, const Lv2cRectangle &r1, const Lv2cRectangle &r2)
{
    double v0 = 1 - value;
    return Lv2cRectangle(
        r1.Left() * v0 + r2.Left() * value,
        r1.Top() * v0 + r2.Top() * value,
        r1.Width() * v0 + r2.Width() * value,
        r1.Height() * v0 + r2.Height() * value);
}

Lv2cRectangle Lv2cScrollBarElement::GetTrackRectangle() const
{
    if (Orientation() == Lv2cScrollBarOrientation::Vertical)
    {
        // ignore the animation. hit test is on AnimationValue() == 1.0 rendering.
        Lv2cRectangle clientRectangle{this->ClientSize()};
        double thumbRadius = clientRectangle.Width() / 2;
        return Lv2cRectangle(
            clientRectangle.Left(),
            clientRectangle.Top() + thumbRadius,
            clientRectangle.Width(),
            clientRectangle.Height() - 2 * thumbRadius);
    }
    else
    {
        // ignore the animation. hit test is on AnimationValue() == 1.0 rendering.
        Lv2cRectangle clientRectangle{this->ClientSize()};
        double thumbRadius = clientRectangle.Height() / 2;
        return Lv2cRectangle(
            clientRectangle.Left()+thumbRadius,
            clientRectangle.Top(),
            clientRectangle.Width() -2*thumbRadius,
            clientRectangle.Height());
    }
    
}

Lv2cRectangle Lv2cScrollBarElement::GetThumbHitRect() const
{
    if (Orientation() == Lv2cScrollBarOrientation::Vertical)
    {
        // ignore the animation. hit test is on AnimationValue() == 1.0 rendering.
        Lv2cRectangle clientRectangle{this->ClientSize()};
        const Lv2cRectangle &paddingRectangle = ClientPaddingRectangle();
        double thumbRadius = clientRectangle.Width() / 2;
        double minY = clientRectangle.Top() + thumbRadius;
        double maxY = clientRectangle.Bottom() - thumbRadius;

        double scrollBottom = WindowSize() + ScrollOffset();
        if (scrollBottom > DocumentSize())
        {
            scrollBottom = DocumentSize();
        }
        double scrollTop = scrollBottom - WindowSize();

        double thumbTop = scrollTop / DocumentSize() * (maxY - minY) + minY;
        double thumbBottom = scrollBottom / DocumentSize() * (maxY - minY) + minY;
        return Lv2cRectangle(
            paddingRectangle.Left(), thumbTop - thumbRadius,
            paddingRectangle.Width(), thumbBottom - thumbTop + thumbRadius * 2);
    }
    else
    {
        // ignore the animation. hit test is on AnimationValue() == 1.0 rendering.
        Lv2cRectangle clientRectangle{this->ClientSize()};
        const Lv2cRectangle &paddingRectangle = ClientPaddingRectangle();
        double thumbRadius = clientRectangle.Height() / 2;
        double minX = clientRectangle.Left() + thumbRadius;
        double maxX = clientRectangle.Right() - thumbRadius;

        double scrollRight = WindowSize() + ScrollOffset();
        if (scrollRight > DocumentSize())
        {
            scrollRight = DocumentSize();
        }
        double scrollLeft = scrollRight - WindowSize();

        double thumbLeft = scrollLeft / DocumentSize() * (maxX - minX) + minX;
        double thumbRight = scrollRight / DocumentSize() * (maxX - minX) + minX;
        return Lv2cRectangle(
            thumbLeft-thumbRadius,paddingRectangle.Top(),
            thumbLeft-thumbRight+thumbRadius*2,
            paddingRectangle.Bottom()
            );
    }
}

void Lv2cScrollBarElement::OnDraw(Lv2cDrawingContext &dc)
{
    Lv2cRectangle clientRectangle{this->ClientSize()};
    Lv2cRectangle paddingRectangle = ClientPaddingRectangle();

    if (WindowSize() >= DocumentSize() || DocumentSize() == 0)
    {
        return;
    }
    if (Orientation() == Lv2cScrollBarOrientation::Vertical)
    {
        // double thumbWidth = trackWidth - padding.left - padding.right;
        Lv2cRectangle trackRect = clientRectangle;

        Lv2cStyle *collapsedStyle = GetCollapsedStyle();

        Lv2cRectangle collapsedTrackRect = Lv2cRectangle(
            paddingRectangle.Right() - collapsedStyle->Width().PixelValue(),
            paddingRectangle.Top(),
            collapsedStyle->Width().PixelValue(),
            paddingRectangle.Height());
        trackRect = BlendRect(AnimationValue(), collapsedTrackRect, paddingRectangle);

        Lv2cRoundCorners roundCorners = Style().RoundCorners().PixelValue();

        Lv2cColor trackColor = Lv2cColor::Blend(
            AnimationValue(),
            TrackHoverColors().GetColor(Lv2cHoverState::Empty),
            TrackHoverColors().GetColor(HoverState()));
        dc.round_corner_rectangle(trackRect, roundCorners);
        dc.set_source(trackColor);
        dc.fill();

        Lv2cThickness collapsedPadding = collapsedStyle->Padding().PixelValue();
        Lv2cRectangle collapsedThumbTrackRect = Lv2cRectangle(
            collapsedTrackRect.Left() + collapsedPadding.left,
            collapsedTrackRect.Top() + collapsedPadding.top,
            collapsedTrackRect.Width() - collapsedPadding.left - collapsedPadding.right,
            collapsedTrackRect.Height() - collapsedPadding.top - collapsedPadding.bottom);

        Lv2cRectangle thumbTrackRect = BlendRect(AnimationValue(), collapsedThumbTrackRect, clientRectangle);
        double thumbRadius = thumbTrackRect.Width() / 2;
        double minY = thumbTrackRect.Top() + thumbRadius;
        double maxY = thumbTrackRect.Bottom() - thumbRadius;

        double scrollBottom = WindowSize() + ScrollOffset();
        if (scrollBottom > DocumentSize())
        {
            scrollBottom = DocumentSize();
        }
        double scrollTop = scrollBottom - WindowSize();

        double thumbTop = scrollTop / DocumentSize() * (maxY - minY) + minY;
        double thumbBottom = scrollBottom / DocumentSize() * (maxY - minY) + minY;
        Lv2cRectangle thumbRect{
            thumbTrackRect.Left(),
            thumbTop - thumbRadius,
            thumbTrackRect.Width(),
            thumbBottom - thumbTop + 2 * thumbRadius};

        Lv2cColor thumbColor = Lv2cColor::Blend(
            AnimationValue(),
            ThumbHoverColors().GetColor(Lv2cHoverState::Empty),
            ThumbHoverColors().GetColor(HoverState()));

        double flubberFactor = FlubberFactor();
        if (flubberFactor != 1)
        {
            dc.save();
            if (flubberFactor < 0)
            {
                dc.translate(thumbRect.Left(),thumbRect.Top());
                dc.scale(1.0,-flubberFactor);
                dc.translate(-thumbRect.Left(),-thumbRect.Top());
            } else {
                dc.translate(thumbRect.Right(),thumbRect.Bottom());
                dc.scale(1.0,flubberFactor);
                dc.translate(-thumbRect.Right(),-thumbRect.Bottom());
            }
        }
        dc.round_corner_rectangle(
            thumbRect,
            Lv2cRoundCorners{thumbRadius, thumbRadius, thumbRadius, thumbRadius});
        dc.set_source(thumbColor);
        dc.fill();

        if (flubberFactor != 1)
        {
            dc.restore();
        }
    }
    else
    {
        // double thumbWidth = trackWidth - padding.left - padding.right;
        Lv2cRectangle trackRect = clientRectangle;

        Lv2cStyle *collapsedStyle = GetCollapsedStyle();

        Lv2cRectangle collapsedTrackRect = Lv2cRectangle(
            paddingRectangle.Left(),
            paddingRectangle.Bottom()- collapsedStyle->Height().PixelValue(),
            paddingRectangle.Width(),
            collapsedStyle->Height().PixelValue()
        );
        trackRect = BlendRect(AnimationValue(), collapsedTrackRect, paddingRectangle);

        Lv2cRoundCorners roundCorners = Style().RoundCorners().PixelValue();

        Lv2cColor trackColor = Lv2cColor::Blend(
            AnimationValue(),
            TrackHoverColors().GetColor(Lv2cHoverState::Empty),
            TrackHoverColors().GetColor(HoverState()));
        dc.round_corner_rectangle(trackRect, roundCorners);
        dc.set_source(trackColor);
        dc.fill();

        Lv2cThickness collapsedPadding = collapsedStyle->Padding().PixelValue();
        Lv2cRectangle collapsedThumbTrackRect = Lv2cRectangle(
            collapsedTrackRect.Left() + collapsedPadding.left,
            collapsedTrackRect.Top() + collapsedPadding.top,
            collapsedTrackRect.Width() - collapsedPadding.left - collapsedPadding.right,
            collapsedTrackRect.Height() - collapsedPadding.top - collapsedPadding.bottom);

        Lv2cRectangle thumbTrackRect = BlendRect(AnimationValue(), collapsedThumbTrackRect, clientRectangle);
        double thumbRadius = thumbTrackRect.Height() / 2;
        double minX = thumbTrackRect.Left() + thumbRadius;
        double maxX = thumbTrackRect.Right() - thumbRadius;

        double scrollRight = WindowSize() + ScrollOffset();
        if (scrollRight > DocumentSize())
        {
            scrollRight = DocumentSize();
        }
        double scrollLeft = scrollRight - WindowSize();

        double thumbLeft = scrollLeft/ DocumentSize() * (maxX - minX) + minX;
        double thumbRight = scrollRight / DocumentSize() * (maxX - minX) + minX;
        Lv2cRectangle thumbRect{
            thumbLeft-thumbRadius,
            thumbTrackRect.Top(),
            thumbRight -thumbLeft + 2*thumbRadius,
            thumbTrackRect.Height()};

        Lv2cColor thumbColor = Lv2cColor::Blend(
            AnimationValue(),
            ThumbHoverColors().GetColor(Lv2cHoverState::Empty),
            ThumbHoverColors().GetColor(HoverState()));

        double flubberFactor = FlubberFactor();
        if (flubberFactor != 1)
        {
            dc.save();
            if (flubberFactor < 0)
            {
                dc.translate(thumbRect.Left(),thumbRect.Top());
                dc.scale(-flubberFactor,1.0);
                dc.translate(-thumbRect.Left(),-thumbRect.Top());
            } else {
                dc.translate(thumbRect.Right(),thumbRect.Bottom());
                dc.scale(flubberFactor,1.0);
                dc.translate(-thumbRect.Right(),-thumbRect.Bottom());
            }
        }
        dc.round_corner_rectangle(
            thumbRect,
            Lv2cRoundCorners{thumbRadius, thumbRadius, thumbRadius, thumbRadius});
        dc.set_source(thumbColor);
        dc.fill();

        if (flubberFactor != 1)
        {
            dc.restore();
        }
    }
}

bool Lv2cScrollBarElement::OnMouseOver(Lv2cMouseOverEventArgs &event)
{
    StartAnimation(1.0);
    return true;
}
bool Lv2cScrollBarElement::OnMouseOut(Lv2cMouseOverEventArgs &event)
{
    animationHoldTime = 1.0;
    StartAnimation(0.0);
    return false;
}

constexpr int MAX_OVERSCROLL = 900;

static double flubberFn(double value)
{
    // double y = -2 * x * x * x + 3 * x * x;
    return std::atan(value/80)/(4*std::numbers::pi);
}
double Lv2cScrollBarElement::ComputeFlubberFactor(double overscroll)
{
    if (overscroll < 0)
    {
        if (overscroll < -MAX_OVERSCROLL)
        {
            overscroll = -MAX_OVERSCROLL;
        }
        double x = overscroll;
        double y = flubberFn(-x);
        return (-(1.0 - y));
    }
    else  if (overscroll > 0)
    {
        if (overscroll > MAX_OVERSCROLL)
        {
            overscroll = MAX_OVERSCROLL;
        }
        double x = overscroll;
        double y = flubberFn(x);
        return ((1.0 - y));
    }
    else
    {
        return 1.0;
    }

    // f(x) = -2a x^3 + 3a x^2.
}
void Lv2cScrollBarElement::AnimationTick(const animation_clock_time_point_t &now)
{
    animationHandle = AnimationHandle::InvalidHandle;

    using namespace std::chrono;
    constexpr double SECONDS_PER_TICK = 1.0 / duration_cast<animation_clock_t::duration>(1000ms).count();

    double dtSeconds = (now - lastAnimationTime).count() * SECONDS_PER_TICK;

    lastAnimationTime = now;

    bool expandoAnimating = false;
    if (this->animationTarget < AnimationValue())
    {
        constexpr double DECREASE_TIME = 0.10;
        constexpr double DECREASE_RATE = 1 / DECREASE_TIME; // full excursion in 0.25 sec.
        if (animationHoldTime != 0)
        {
            expandoAnimating = true;
            if (!mouseDown)
            {
                animationHoldTime -= dtSeconds;
                if (animationHoldTime < 0)
                {
                    dtSeconds = -animationHoldTime;
                    animationHoldTime = 0;
                }
            }
        }
        if (animationHoldTime == 0)
        {
            double dx = -dtSeconds * DECREASE_RATE;
            double v = AnimationValue() + dx;
            if (v <= 0)
            {
                v = 0;
                AnimationValue(0);
            }
            else
            {
                AnimationValue(v);
                expandoAnimating = true;
            }
        }
    }
    else if (this->animationTarget > AnimationValue())
    {
        constexpr double INCREASE_TIME = 0.1;
        constexpr double INCREASE_RATE = 1 / INCREASE_TIME; // full excursion in 0.25 sec.
        double dx = dtSeconds * INCREASE_RATE;
        double v = AnimationValue() + dx;
        if (v >= 1.0)
        {
            AnimationValue(1.0);
        }
        else
        {
            AnimationValue(v);
            expandoAnimating = true;
        }
    }
    bool flubberAnimating = false;

    if (mouseDown)
    {
        flubberAnimating = true;
        double overscroll = OverscrollAmount();
        if (overscroll > MAX_OVERSCROLL)
        {
            overscroll = MAX_OVERSCROLL;
        }
        if (overscroll < -MAX_OVERSCROLL)
        {
            overscroll = -MAX_OVERSCROLL;
        }

        animationOverscroll = overscroll;
        double flubber = ComputeFlubberFactor(overscroll);
        FlubberFactor(flubber);
    } else {
        if (animationOverscroll != 0)
        {
            constexpr double DECREASE_TIME = 0.16; // full excursion in seconds
            constexpr double DECREASE_RATE = 1 / DECREASE_TIME; // 
            double dx = dtSeconds*DECREASE_RATE*MAX_OVERSCROLL;

            if (animationOverscroll > 0)
            {
                animationOverscroll -= dx;
                if (animationOverscroll < 0)
                {
                    animationOverscroll = 0;
                    FlubberFactor(1.0);
                } else {
                    double flubber = ComputeFlubberFactor(animationOverscroll);
                    FlubberFactor(flubber);
                    flubberAnimating = true;
                }
            } else {
                animationOverscroll += dx;
                if (animationOverscroll > 0)
                {
                    animationOverscroll = 0;
                    FlubberFactor(1.0);
                } else {
                    double flubber = ComputeFlubberFactor(animationOverscroll);
                    FlubberFactor(flubber);
                    flubberAnimating = true;
                }
            }
        }
    }

    if (expandoAnimating || flubberAnimating)
    {
        this->animationHandle = Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t &now)
            {
                AnimationTick(now);
            });
    }
}
void Lv2cScrollBarElement::StartAnimation(double targetValue)
{
    this->animationTarget = targetValue;
    this->lastAnimationTime = animation_clock_t::now();
    if (!animationHandle)
    {
        animationHandle = Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t &now)
            {
                CheckValid();
                AnimationTick(now);
            });
    }
}
void Lv2cScrollBarElement::StopAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
}

void Lv2cScrollBarElement::OnMount()
{
    this->ClearClasses();
    super::OnMount();
    auto &theme = Theme();
    this->ThumbHoverColors(theme.scrollbarThumbColors);
    this->TrackHoverColors(theme.scrollbarTrackColors);
    this->AddClass(
        Orientation() == Lv2cScrollBarOrientation::Horizontal
            ? theme.horizontalScrollbarStyle
            : theme.verticalScrollbarStyle);
}
void Lv2cScrollBarElement::OnUnmount()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
    AnimationValue(0);
    FlubberFactor(0);
    super::OnUnmount();
}

void Lv2cScrollBarElement::OnHoverStateChanged(Lv2cHoverState hoverState)
{
    super::OnHoverStateChanged(hoverState);
    Invalidate();
}

Lv2cStyle *Lv2cScrollBarElement::GetCollapsedStyle() const
{
    if (CollapsedStyle())
    {
        return CollapsedStyle().get();
    }
    if (Orientation() == Lv2cScrollBarOrientation::Horizontal)
    {
        return Theme().collapsedHorizontalScrollbarStyle.get();
    }
    else
    {
        return Theme().collapsedVerticalScrollbarStyle.get();
    }
}

bool Lv2cScrollBarElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    if (!this->CaptureMouse())
    {
        return false;
    }
    // hold the expando animation.
    this->mouseDown = true;
    StartAnimation(animationTarget);
    animationHoldTime = 1.0;

    HoverState(HoverState() + Lv2cHoverState::Pressed);
    // Lv2cRectangle mouseHitRect = GetThumbHitRect();
    // isMouseDragging = mouseHitRect.Contains(event.point);
    isMouseDragging = false;
    this->mouseDownOffset = ScrollOffset();
    this->mouseDownPoint = event.screenPoint;
    return true;
}

bool Lv2cScrollBarElement::OnMouseUp(Lv2cMouseEventArgs &event)
{
    this->mouseDown = false;
    OverscrollAmount(0);
    HoverState(HoverState() - Lv2cHoverState::Pressed);


    if (this->Capture() == this)
    {
        if (!isMouseDragging)
        {
            // it's a click.
            // Center the window on the point that was clicked.
            Lv2cRectangle trackRectangle = this->GetTrackRectangle();
            if (this->Orientation() == Lv2cScrollBarOrientation::Vertical)
            {
                double y = (event.point.y-trackRectangle.Top())/trackRectangle.Height()*DocumentSize();
                y -= WindowSize()/2;
                y = std::round(y);
                if (y > DocumentSize()-WindowSize())
                {
                    y = DocumentSize()-WindowSize();
                }
                if (y < 0) y =0;
                ScrollOffset(y);
            } else {
                double x = (event.point.x-trackRectangle.Left())/trackRectangle.Width()*DocumentSize();
                x -= WindowSize()/2;
                x = std::round(x);
                if (x > DocumentSize()-WindowSize())
                {
                    x = DocumentSize()-WindowSize();
                }
                if (x < 0) x =0;
                ScrollOffset(x);
            }
        }
        ReleaseCapture();
        return true;
    }
    return false;
}
bool Lv2cScrollBarElement::OnMouseMove(Lv2cMouseEventArgs &event)
{
    if (Lv2cPoint::Distance(event.screenPoint, mouseDownPoint) >= 2.4)
    {
        isMouseDragging = true;
    }
    if (isMouseDragging)
    {
        Lv2cRectangle trackRectangle = GetTrackRectangle();
        double scrollOffset;
        if (Orientation() == Lv2cScrollBarOrientation::Vertical)
        {
            double dy = event.screenPoint.y - this->mouseDownPoint.y;

            // pixels to scroll offset.
            double dScrollOffset = dy * this->DocumentSize() / trackRectangle.Height();
            scrollOffset = mouseDownOffset + dScrollOffset;
        } else {
            double dx = event.screenPoint.x - this->mouseDownPoint.x;

            // pixels to scroll offset.
            double dScrollOffset = dx * this->DocumentSize() / trackRectangle.Width();
            scrollOffset = mouseDownOffset + dScrollOffset;

        }
        
        if (scrollOffset > DocumentSize() - WindowSize())
        {
            OverscrollAmount(scrollOffset - (DocumentSize() - WindowSize()));
            scrollOffset = DocumentSize() - WindowSize();
        } else if (scrollOffset < 0)
        {
            OverscrollAmount(scrollOffset);
            scrollOffset = 0;
        } else {
            OverscrollAmount(0);
        }
        ScrollOffset(scrollOffset);
    }
    return true;
}
