#include "lvtk/LvtkSwitchElement.hpp"
#include "lvtk/LvtkDropShadowElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include <numbers>

using namespace lvtk;

namespace lvtk
{
    class LvtkToggleTrackElement : public LvtkElement
    {
    public:
        using self = LvtkToggleTrackElement;
        using super = LvtkElement;
        using ptr = std::shared_ptr<self>;
        LvtkToggleTrackElement();
        static ptr Create() { return std::make_shared<self>(); }

        BINDING_PROPERTY(Position, double, 0.0)

        void IsOnOff(bool value) { isOnOff = value; }
        bool IsOnOff() const { return isOnOff; }

    protected:
        virtual void OnMount() override;
        virtual bool WillDraw() const override { return true; }
        virtual void OnDraw(LvtkDrawingContext &dc) override;

    private:
        bool isOnOff = false;
    };
    class LvtkToggleThumbElement : public LvtkElement
    {
    public:
        using self = LvtkToggleThumbElement;
        using super = LvtkElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }

        LvtkToggleThumbElement();

        BINDING_PROPERTY(Position, double, 0.0)
        BINDING_PROPERTY(Pressed, double, 0.0)

        void IsOnOff(bool value) { isOnOff = value; }
        bool IsOnOff() const { return isOnOff; }

    protected:
        virtual void OnMount() override;
        virtual bool WillDraw() const override { return true; }
        virtual void OnDraw(LvtkDrawingContext &dc) override;

    private:
        bool isOnOff = false;
    };

}

void LvtkSwitchElement::OnMount()
{
    this->Classes(Theme().toggleButtonStyle);
    thumb->IsOnOff(this->IsOnOff());
    track->IsOnOff(this->IsOnOff());
    this->thumbShadow->DropShadow(Theme().toggleThumbDropShadow);
    this->trackShadow->DropShadow(Theme().toggleTrackDropShadow);
}

void LvtkSwitchElement::OnUnmount()
{
    StopAnimation();
    Position(Checked() ? 1.0 : 0.0);
    super::OnUnmount();
}

void LvtkSwitchElement::AnimationTick(const animation_clock_time_point_t&now)
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

void LvtkSwitchElement::StopAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
        double target = Checked() ? 1 : 0;
        Position(target);
    }
}
void LvtkSwitchElement::StartAnimation()
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
void LvtkSwitchElement::OnValueChanged(double value)
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

void LvtkSwitchElement::OnDraw(LvtkDrawingContext &dc)
{
    // LvtkRectangle clientRectangle(this->ClientSize());
    // if (HoverState() != LvtkHoverState::Empty)
    // {
    //     LvtkRectangle borderRect{
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

bool LvtkSwitchElement::OnClick(LvtkMouseEventArgs &event)
{
    Checked(!Checked());
    return true;
}

bool LvtkSwitchElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    StopAnimation();
    this->CaptureMouse();
    HoverState(HoverState() + LvtkHoverState::Pressed);
    dragging = false;
    startPosition = Position();
    startPoint = event.screenPoint;
    Pressed(true);
    return true;
}
bool LvtkSwitchElement::OnMouseUp(LvtkMouseEventArgs &event)
{
    if (this->Capture() == this)
    {
        this->ReleaseCapture();
        HoverState(HoverState() - LvtkHoverState::Pressed);
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
bool LvtkSwitchElement::OnMouseMove(LvtkMouseEventArgs &event)
{
    if (this->Capture() == this)
    {
        if (LvtkPoint::Distance(this->startPoint, event.screenPoint) > 3)
        {
            dragging = true;
            HoverState(HoverState() + LvtkHoverState::Pressed);
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
                HoverState(HoverState() + LvtkHoverState::Pressed);
                Pressed(true);
            }
            else
            {
                HoverState(HoverState() - LvtkHoverState::Pressed);
            }
        }
        return true;
    }
    return false;
}
void LvtkSwitchElement::OnHoverStateChanged(LvtkHoverState hoverState)
{
    Invalidate();

    double opacity = Theme().dialHoverOpacity.GetOpacity(hoverState);
    trackShadow->Style().Opacity(opacity);
}

void LvtkToggleTrackElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    LvtkRectangle clientRect{this->ClientSize()};

    double offset = Position() * (clientRect.Width() - clientRect.Height());
    auto roundCorners = this->Style().RoundCorners().PixelValue();

    if (IsOnOff())
    {
        double x = offset + clientRect.Height() / 2;
        LvtkRectangle rcLeft{0, 0, x, clientRect.Height()};
        dc.round_corner_rectangle(
            rcLeft,
            LvtkRoundCorners{
                roundCorners.topLeft, 0,
                roundCorners.bottomLeft, 0});

        dc.set_source(Style().Color());
        dc.fill();

        LvtkRectangle rcRight{x, 0, clientRect.Width() - x, clientRect.Height()};
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
LvtkToggleTrackElement::LvtkToggleTrackElement()
{
    PositionProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
}
void LvtkToggleTrackElement::OnMount()
{
    this->ClearClasses();
    super::OnMount();
    this->AddClass({Theme().toggleTrackStyle});
}

void LvtkToggleThumbElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    LvtkRectangle clientRect{this->ClientSize()};
    double offset = Position() * (clientRect.Width() - clientRect.Height());
    auto roundCorners = this->Style().RoundCorners().PixelValue();
    LvtkRectangle thumbRect{offset, 0, clientRect.Height(), clientRect.Height()};
    if (!Pressed())
    {
        thumbRect = thumbRect.Inflate(-1, -1, -1, -1);
        roundCorners.bottomLeft--;
        roundCorners.topLeft--;
        roundCorners.topRight--;
        roundCorners.bottomRight--;
    }
    dc.round_corner_rectangle(thumbRect, roundCorners);
    LvtkColor thumbColor = Style().Color().get_color();
    if (IsOnOff())
    {
        LvtkColor disabledColor = Theme().toggleButtonOffTrackColor;
        thumbColor = LvtkColor::LinearBlend(Position(), disabledColor, thumbColor);
    }
    dc.set_source(thumbColor);
    dc.fill();
}
LvtkToggleThumbElement::LvtkToggleThumbElement()
{
    PositionProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
    PressedProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
}

void LvtkToggleThumbElement::OnMount()
{
    this->Classes({Theme().toggleThumbStyle});
}

LvtkSwitchElement::LvtkSwitchElement()
{
    track = LvtkToggleTrackElement::Create();
    thumb = LvtkToggleThumbElement::Create();

    trackShadow = LvtkDropShadowElement::Create();
    trackShadow->Style()
        .Opacity(0.75)
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);
    AddChild(trackShadow);
    trackShadow->AddChild(track);

    thumbShadow = LvtkDropShadowElement::Create();
    AddChild(thumbShadow);
    thumbShadow->AddChild(thumb);

    thumbShadow->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);

    PositionProperty.Bind(track->PositionProperty);
    PositionProperty.Bind(thumb->PositionProperty);
}