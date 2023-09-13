#include "lvtk/LvtkDbVuElement.hpp"
#include "lvtk/LvtkWindow.hpp"

using namespace lvtk;

using namespace std::chrono;
constexpr milliseconds HOLD_TIME = 2000ms;



void LvtkDbVuElement::UpdateStyle()
{
    Classes(Theme().dbVuStyle);
    InvalidateLayout();
}
void LvtkStereoDbVuElement::UpdateStyle()
{
    Classes(Theme().stereoDbVuStyle);
    InvalidateLayout();
}
const LvtkVuSettings &LvtkDbVuElement::Settings() const
{
    return Theme().dbVuSettings;
}
const LvtkVuSettings &LvtkStereoDbVuElement::Settings() const
{
    return Theme().dbVuSettings;
}
void LvtkDbVuElement::DrawTicks(
    LvtkDrawingContext &dc,
    double minValue,
    double maxValue,
    const LvtkVuSettings &settings,
    const LvtkRectangle &vuRectangle,
    LvtkColor color)

{
    // draw the zero tick.
    double y = LvtkVuElement::ValueToClient(0,minValue,maxValue, vuRectangle);
    dc.rectangle(settings.padding, y, settings.tickWidth, 1);
    dc.set_source(color);
    dc.fill();

    double tickDb = settings.tickDb;

    LvtkColor minorTickColor = LvtkColor(color, 0.60);
    dc.set_source(minorTickColor);

    while (true)
    {
        if (maxValue == minValue)
            break;
        if (vuRectangle.Height() <= 0)
            break;
        double tickPixels = (tickDb)*vuRectangle.Height() / (maxValue-minValue);
        if (tickPixels > 5)
        {
            break;
        }
        tickDb *= 2;
    }
    for (double db = tickDb; db < maxValue; db += tickDb)
    {
        y = LvtkVuElement::ValueToClient(db,minValue,maxValue, vuRectangle);
        dc.rectangle(settings.padding, y, settings.tickWidth, 1);
        dc.fill();
    }
    for (double db = -tickDb; db > minValue; db -= tickDb)
    {
        y = LvtkVuElement::ValueToClient(db,minValue,maxValue, vuRectangle);
        dc.rectangle(settings.padding, y, settings.tickWidth, 1);
        dc.fill();
    }
}

/* static*/
void LvtkDbVuElement::DrawTelltale(
    LvtkDrawingContext &dc,
    double telltaleValue,
    double value,
    double minValue,
    double maxValue,
    const LvtkVuSettings &settings,
    const LvtkRectangle &vuBarRectangle)
{
    double y = LvtkVuElement::ValueToClient(telltaleValue, minValue, maxValue, vuBarRectangle);
    double yValue = LvtkVuElement::ValueToClient(value, minValue, maxValue, vuBarRectangle);
    if (std::abs(y - yValue) > 1)
    {
        double height = 1;
        LvtkColor color = settings.green;
        if (settings.yellowLevel.has_value())
        {
            double yYellow = LvtkVuElement::ValueToClient(settings.yellowLevel.value(), minValue, maxValue, vuBarRectangle);
            if (y < yYellow)
            {
                color = settings.yellow;
            }
        }
        if (settings.redLevel.has_value())
        {
            double yRed = LvtkVuElement::ValueToClient(settings.redLevel.value(), minValue, maxValue, vuBarRectangle);
            if (y < yRed)
            {
                color = settings.red;
            }
            height = 2;
        }
        dc.set_source(color);
        dc.rectangle(
            vuBarRectangle.Left(),
            y,
            vuBarRectangle.Width(),
            height);
        dc.fill();
    }
}

void LvtkDbVuElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc); // draws the vus,

    LvtkRectangle clientRectangle{ClientSize()};

    auto &settings = Settings();
    LvtkRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    DrawTicks(
        dc,
        MinValue(),MaxValue(),
        Settings(),
        vuRectangle,
        Theme().vuTickColor);

    // draw the telltale
    if (HoldValue() != Value())
    {
        double offsetX = settings.tickWidth + settings.padding;
        LvtkRectangle vuBarRectangle = LvtkRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());

        DrawTelltale(dc,HoldValue(),Value(),MinValue(),MaxValue(), Settings(), vuBarRectangle);
    }
}

void LvtkStereoDbVuElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc); // draws the vus,

    LvtkRectangle clientRectangle{ClientSize()};

    auto &settings = Settings();
    LvtkRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    LvtkDbVuElement::DrawTicks(
        dc,
        MinValue(),MaxValue(),
        Settings(),
        vuRectangle,
        Theme().vuTickColor);

    // draw the telltale
    double offsetX = settings.tickWidth + settings.padding;
    LvtkRectangle vuBoxRectangle = LvtkRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());
    double vuWidth = (vuBoxRectangle.Width()-settings.padding)/2;
    if (HoldValue() != Value())
    {
        LvtkRectangle vuLeft { vuBoxRectangle.Left(),vuBoxRectangle.Top(),vuWidth,vuBoxRectangle.Height()};
        LvtkDbVuElement::DrawTelltale(dc,HoldValue(),Value(),MinValue(),MaxValue(), Settings(), vuLeft);
    }
    if (RightHoldValue() != RightValue())
    {
        LvtkRectangle vuRight { vuBoxRectangle.Right()-vuWidth,vuBoxRectangle.Top(),vuWidth,vuBoxRectangle.Height()};
        LvtkDbVuElement::DrawTelltale(dc,RightHoldValue(),RightValue(),MinValue(),MaxValue(), Settings(), vuRight);
    }
}


LvtkDbVuElement::LvtkDbVuElement()
{
    HoldValueProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
}
LvtkStereoDbVuElement::LvtkStereoDbVuElement()
{
    leftAnimationStartTime = rightAnimationStartTime = clock_t::now();

    HoldValueProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
    RightHoldValueProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
}

void LvtkDbVuElement::OnMount()
{
    super::OnMount();
    HoldValue(Value());
    StopAnimation();
}

void LvtkStereoDbVuElement::OnMount()
{
    super::OnMount();
    HoldValue(Value());
    RightHoldValue(RightValue());
    StopAnimation();
}

void LvtkDbVuElement::OnUnmount()
{
    StopAnimation();
}
void LvtkStereoDbVuElement::OnUnmount()
{
    StopAnimation();
}
void LvtkDbVuElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
    if (IsMounted())
    {
        if (value > HoldValue())
        {
            HoldValue(value);
            StartAnimation(true);
        } else {
            StartAnimation(false);
        }
    } else {
        HoldValue(value);
    }
}
void LvtkStereoDbVuElement::OnValueChanged(double value)
{
    super::OnValueChanged(value);
    if (IsMounted())
    {
        if (value > HoldValue())
        {
            HoldValue(value);
            StartLeftAnimation(true);
        } else {
            StartLeftAnimation(false);
        }
    } else {
        HoldValue(value);        
    }
}
void LvtkStereoDbVuElement::OnRightValueChanged(double value)
{
    super::OnValueChanged(value);
    if (IsMounted())
    {
        if (value > RightHoldValue())
        {
            RightHoldValue(value);
            StartRightAnimation(true);
        } else {
            if (rightAnimationActive)
            {
                return;
            }
            StartRightAnimation(false);
        }
    } else {
        RightHoldValue(value);
    }
}


void LvtkDbVuElement::StopAnimation()
{
    if (animationHandle)
    {
        this->Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
}
void LvtkStereoDbVuElement::StopAnimation()
{
    if (animationHandle)
    {
        this->Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
    rightAnimationActive = false;
    leftAnimationActive = false;
}

void LvtkDbVuElement::AnimationTick(const animation_clock_time_point_t&now)
{
    animationHandle = AnimationHandle::InvalidHandle;
    constexpr double HOLD_RATE = 1.0; // full exursion in seconds.
    using namespace std::chrono;
    constexpr clock_t::rep TICKS_PER_SECOND = duration_cast<clock_t::duration>(1000ms).count();
    constexpr double TICKS_TO_SECONDS = 1.0 / TICKS_PER_SECOND;

    double seconds = (now - animationStartTime).count() * TICKS_TO_SECONDS;
    if (seconds > 0)
    {
        double decayTime = seconds;
        double newValue = animationStartValue - decayTime / HOLD_RATE * (MaxValue() - MinValue());
        if (newValue < Value())
        {
            HoldValue(Value());
            animationHandle = AnimationHandle::InvalidHandle;
            return; // all done, no more animation.
        }
        HoldValue(newValue);
    }
    if (Window())
    {
        animationHandle = Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                AnimationTick(now);
            });
    }
}
void LvtkStereoDbVuElement::AnimationTick(const animation_clock_time_point_t&now)
{
    using namespace std::chrono;

    animationHandle = AnimationHandle::InvalidHandle;
    constexpr double HOLD_RATE = 1.0; // full exursion in seconds.
    constexpr clock_t::rep TICKS_PER_SECOND = duration_cast<clock_t::duration>(1000ms).count();
    constexpr double TICKS_TO_SECONDS = 1.0 / TICKS_PER_SECOND;

    bool animated = false;
    {
        if (leftAnimationActive)
        {
            double seconds = (now - leftAnimationStartTime).count() * TICKS_TO_SECONDS;
            if (seconds <= 0)
            {
                animated = true;
            }
            else 
            {
                double decayTime = seconds;
                double newValue = leftAnimationStartValue - decayTime / HOLD_RATE * (MaxValue() - MinValue());
                if (newValue < Value())
                {
                    HoldValue(Value());
                    leftAnimationActive = false;
                } else {
                    animated = true;
                    HoldValue(newValue);
                }
            }
        }
    }
    {
        if (rightAnimationActive)
        {
            double seconds = (now - rightAnimationStartTime).count() * TICKS_TO_SECONDS;
            if (seconds <= 0)
            {
                animated = true;
            } else 
            {
                double decayTime = seconds;
                double newValue = rightAnimationStartValue - decayTime / HOLD_RATE * (MaxValue() - MinValue());
                if (newValue < RightValue())
                {
                    rightAnimationActive = false;
                    RightHoldValue(RightValue());
                } else {
                    animated = true;
                    RightHoldValue(newValue);
                }
            }
        }
    }

    if (animated && Window())
    {
        animationHandle = Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                AnimationTick(now);
            });
    } else {
        animationHandle = AnimationHandle::InvalidHandle;
    }
}

void LvtkDbVuElement::StartAnimation(bool hold)
{
    using namespace std::chrono;
    if (hold) {
        animationStartTime = clock_t::now() + duration_cast<clock_t::duration>(HOLD_TIME);
    } else {
        if (animationHandle) 
        {
            return;
        }
        animationStartTime = clock_t::now();
    }
    animationStartValue = HoldValue();
    if (!animationHandle)
    {
        this->Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                AnimationTick(now);
            });
    }
}
void LvtkStereoDbVuElement::StartLeftAnimation(bool hold)
{
    using namespace std::chrono;
    if (hold) {
        leftAnimationStartTime = clock_t::now() + duration_cast<clock_t::duration>(HOLD_TIME);
    } else {
        if (leftAnimationActive) 
        {
            return;
        }
        leftAnimationStartTime = clock_t::now();
    }
    leftAnimationStartValue = HoldValue();
    leftAnimationActive = true;
    if (!animationHandle)
    {
        this->Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                AnimationTick(now);
            });
    }
}

void LvtkStereoDbVuElement::StartRightAnimation(bool hold)
{
    using namespace std::chrono;
    if (hold) {
        rightAnimationStartTime = clock_t::now() + duration_cast<clock_t::duration>(HOLD_TIME);
    } else {
        rightAnimationStartTime = clock_t::now();
    }
    rightAnimationStartValue = RightHoldValue();
    rightAnimationActive = true;
    if (!animationHandle)
    {
        this->Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t&now)
            {
                AnimationTick(now);
            });
    }
}

double LvtkDbVuElement::ValueToClient(double value, const LvtkRectangle &vuRectangle)
{
    return LvtkVuElement::ValueToClient(value, MinValue(), MaxValue(), vuRectangle);
}
