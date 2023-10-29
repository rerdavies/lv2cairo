#include "lv2c/Lv2cSwitchElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include <numbers>

using namespace lvtk;

namespace lvtk
{
    class Lv2cToggleTrackElement : public Lv2cElement
    {
    public:
        using self = Lv2cToggleTrackElement;
        using super = Lv2cElement;
        using ptr = std::shared_ptr<self>;
        Lv2cToggleTrackElement();
        static ptr Create() { return std::make_shared<self>(); }

        BINDING_PROPERTY(Position, double, 0.0)

        void IsOnOff(bool value) { isOnOff = value; }
        bool IsOnOff() const { return isOnOff; }

    protected:
        virtual void OnMount() override;
        virtual bool WillDraw() const override { return true; }
        virtual void OnDraw(Lv2cDrawingContext &dc) override;

    private:
        bool isOnOff = false;
    };
    class Lv2cToggleThumbElement : public Lv2cElement
    {
    public:
        using self = Lv2cToggleThumbElement;
        using super = Lv2cElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }

        Lv2cToggleThumbElement();

        BINDING_PROPERTY(Position, double, 0.0)
        BINDING_PROPERTY(Pressed, double, 0.0)

        void IsOnOff(bool value) { isOnOff = value; }
        bool IsOnOff() const { return isOnOff; }

    protected:
        virtual void OnMount() override;
        virtual bool WillDraw() const override { return true; }
        virtual void OnDraw(Lv2cDrawingContext &dc) override;

    private:
        bool isOnOff = false;
    };

}

void Lv2cSwitchElement::OnMount()
{
    this->Classes(Theme().toggleButtonStyle);
    thumb->IsOnOff(this->IsOnOff());
    track->IsOnOff(this->IsOnOff());
    this->thumbShadow->DropShadow(Theme().toggleThumbDropShadow);
    this->trackShadow->DropShadow(Theme().toggleTrackDropShadow);
}

void Lv2cSwitchElement::OnUnmount()
{
    StopAnimation();
    Position(Checked() ? 1.0 : 0.0);
    super::OnUnmount();
}

void Lv2cSwitchElement::AnimationTick(const animation_clock_time_point_t&now)
{
    using namespace std::chrono;

    constexpr double SECONDS_PER_TICK = 1.0 / duration_cast<clock_t::duration>(1s).count();

    constexpr double ANIMATION_RATE = 1.0 /*FULL RANGE */ / 0.1 /* seconds for full transition*/;
    bool finished = false;
    if (now != lastAnimationTime)
    {
        clock_t::rep elapsed = (clock_t::now() - lastAnimationTime).count();
        double elapsedSeconds = elapsed * SECONDS_PER_TICK;
        double dPosition = elapsedSeconds * ANIMATION_RATE;

        double target = Checked() ? 1.0 : 0.0;
        double newValue;
        if (Position() < target)
        {
            newValue = Position() + dPosition;
            if (newValue >= target)
            {
                newValue = target;
                finished = true;
            }
        }
        else if (Position() > target)
        {
            newValue = Position() - dPosition;
            if (newValue < 0)
            {
                newValue = 0;
                finished = true;
            }
        }
        else
        {
            return;
        }
        Position(newValue);
    }
    if (!finished)
    {
        // again.
        Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t &now)
            {
                AnimationTick(now);
            });
    }
}

void Lv2cSwitchElement::StopAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
        double target = Checked() ? 1 : 0;
        Position(target);
    }
}
void Lv2cSwitchElement::StartAnimation()
{
    if (animationHandle)
    {
        return; // already animating.
    }
    double target = Checked() ? 1 : 0;
    if (Position() == target)
    {
        return;
    }
    using namespace std::chrono;

    if (!this->Window())
    {
        Position(Checked() ? 1.0 : 0.0);
        Invalidate();
    }
    lastAnimationTime = clock_t::now();
    animationHandle = this->Window()->RequestAnimationCallback(
        [this](const animation_clock_time_point_t&now)
        {
            AnimationTick(now);
        });
}
void Lv2cSwitchElement::OnValueChanged(double value)
{
    if (checked != Checked())
    {
        checked = Checked();
        if (IsMounted())
        {
            StartAnimation();
        }
        else
        {
            Position(Checked() ? 1.0 : 0.0);
        }
    }
}

void Lv2cSwitchElement::OnDraw(Lv2cDrawingContext &dc)
{
    // Lv2cRectangle clientRectangle(this->ClientSize());
    // if (HoverState() != Lv2cHoverState::Empty)
    // {
    //     Lv2cRectangle borderRect{
    //         borderBounds.getLeft() - clientBounds.getLeft(),
    //         borderBounds.getTop() - clientBounds.getTop(),
    //         borderBounds.getWidth(),
    //         borderBounds.getHeight()};
    //     auto roundCornerMeasurement = Style().RoundCorners();
    //     auto roundCorners = roundCornerMeasurement.getPixels();
    //     dc.round_corner_rectangle(borderRect, roundCorners);
    //     auto color = Theme().hoverBackgroundColors.GetColor(HoverState());
    //     dc.set_source(color);
    //     dc.fill();
    // }
}

bool Lv2cSwitchElement::OnClick(Lv2cMouseEventArgs &event)
{
    Checked(!Checked());
    return true;
}

bool Lv2cSwitchElement::OnMouseDown(Lv2cMouseEventArgs &event)
{
    StopAnimation();
    this->CaptureMouse();
    HoverState(HoverState() + Lv2cHoverState::Pressed);
    dragging = false;
    startPosition = Position();
    startPoint = event.screenPoint;
    Pressed(true);
    return true;
}
bool Lv2cSwitchElement::OnMouseUp(Lv2cMouseEventArgs &event)
{
    if (this->Capture() == this)
    {
        this->ReleaseCapture();
        HoverState(HoverState() - Lv2cHoverState::Pressed);
        Pressed(false);
        if (dragging)
        {
            Checked(Position() >= 0.5);
            StartAnimation();
        }
        else
        {
            OnClick(event);
        }
        return true;
    }
    return false;
}
bool Lv2cSwitchElement::OnMouseMove(Lv2cMouseEventArgs &event)
{
    if (this->Capture() == this)
    {
        if (Lv2cPoint::Distance(this->startPoint, event.screenPoint) > 3)
        {
            dragging = true;
            HoverState(HoverState() + Lv2cHoverState::Pressed);
            Pressed(true);
            auto thumbSize = thumb->ClientSize();
            this->trackWidth = thumbSize.Width() - thumbSize.Height();
        }
        if (dragging)
        {
            double position = startPosition + (event.screenPoint.x - this->startPoint.x) / trackWidth;
            if (position > 1)
                position = 1;
            if (position < 0)
                position = 0;
            Position(position);
        }
        else
        {
            if (this->ScreenBounds().Contains(event.screenPoint))
            {
                HoverState(HoverState() + Lv2cHoverState::Pressed);
                Pressed(true);
            }
            else
            {
                HoverState(HoverState() - Lv2cHoverState::Pressed);
            }
        }
        return true;
    }
    return false;
}
void Lv2cSwitchElement::OnHoverStateChanged(Lv2cHoverState hoverState)
{
    Invalidate();

    double opacity = Theme().dialHoverOpacity.GetOpacity(hoverState);
    trackShadow->Style().Opacity(opacity);
}

void Lv2cToggleTrackElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    Lv2cRectangle clientRect{this->ClientSize()};

    double offset = Position() * (clientRect.Width() - clientRect.Height());
    auto roundCorners = this->Style().RoundCorners().PixelValue();

    if (IsOnOff())
    {
        double x = offset + clientRect.Height() / 2;
        Lv2cRectangle rcLeft{0, 0, x, clientRect.Height()};
        dc.round_corner_rectangle(
            rcLeft,
            Lv2cRoundCorners{
                roundCorners.topLeft, 0,
                roundCorners.bottomLeft, 0});

        dc.set_source(Style().Color());
        dc.fill();

        Lv2cRectangle rcRight{x, 0, clientRect.Width() - x, clientRect.Height()};
        dc.round_corner_rectangle(
            rcRight, {0, roundCorners.topRight, 0, roundCorners.bottomRight});
        dc.set_source(Theme().toggleButtonOffThumbColor);
        dc.fill();
    }
    else
    {
        dc.set_source(Style().Color());
        dc.round_corner_rectangle(clientRect, roundCorners);
        dc.fill();
    }
}
Lv2cToggleTrackElement::Lv2cToggleTrackElement()
{
    PositionProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}
void Lv2cToggleTrackElement::OnMount()
{
    this->ClearClasses();
    super::OnMount();
    this->AddClass({Theme().toggleTrackStyle});
}

void Lv2cToggleThumbElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    Lv2cRectangle clientRect{this->ClientSize()};
    double offset = Position() * (clientRect.Width() - clientRect.Height());
    auto roundCorners = this->Style().RoundCorners().PixelValue();
    Lv2cRectangle thumbRect{offset, 0, clientRect.Height(), clientRect.Height()};
    if (!Pressed())
    {
        thumbRect = thumbRect.Inflate(-1, -1, -1, -1);
        roundCorners.bottomLeft--;
        roundCorners.topLeft--;
        roundCorners.topRight--;
        roundCorners.bottomRight--;
    }
    dc.round_corner_rectangle(thumbRect, roundCorners);
    Lv2cColor thumbColor = Style().Color().get_color();
    if (IsOnOff())
    {
        Lv2cColor disabledColor = Theme().toggleButtonOffTrackColor;
        thumbColor = Lv2cColor::LinearBlend(Position(), disabledColor, thumbColor);
    }
    dc.set_source(thumbColor);
    dc.fill();
}
Lv2cToggleThumbElement::Lv2cToggleThumbElement()
{
    PositionProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    PressedProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}

void Lv2cToggleThumbElement::OnMount()
{
    this->Classes({Theme().toggleThumbStyle});
}

Lv2cSwitchElement::Lv2cSwitchElement()
{
    track = Lv2cToggleTrackElement::Create();
    thumb = Lv2cToggleThumbElement::Create();

    trackShadow = Lv2cDropShadowElement::Create();
    trackShadow->Style()
        .Opacity(0.75)
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);
    AddChild(trackShadow);
    trackShadow->AddChild(track);

    thumbShadow = Lv2cDropShadowElement::Create();
    AddChild(thumbShadow);
    thumbShadow->AddChild(thumb);

    thumbShadow->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);

    PositionProperty.Bind(track->PositionProperty);
    PositionProperty.Bind(thumb->PositionProperty);
}