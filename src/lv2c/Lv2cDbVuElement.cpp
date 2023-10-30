#include "lv2c/Lv2cDbVuElement.hpp"
#include "lv2c/Lv2cWindow.hpp"

using namespace lv2c;

using namespace std::chrono;
constexpr milliseconds HOLD_TIME = 2000ms;



void Lv2cDbVuElement::UpdateStyle()
{
    Classes(Theme().dbVuStyle);
    InvalidateLayout();
}
void Lv2cStereoDbVuElement::UpdateStyle()
{
    Classes(Theme().stereoDbVuStyle);
    InvalidateLayout();
}
const Lv2cVuSettings &Lv2cDbVuElement::Settings() const
{
    return Theme().dbVuSettings;
}
const Lv2cVuSettings &Lv2cStereoDbVuElement::Settings() const
{
    return Theme().dbVuSettings;
}
void Lv2cDbVuElement::DrawTicks(
    Lv2cDrawingContext &dc,
    double minValue,
    double maxValue,
    const Lv2cVuSettings &settings,
    const Lv2cRectangle &vuRectangle,
    Lv2cColor color)

{
    // draw the zero tick.
    double y = Lv2cVuElement::ValueToClient(0,minValue,maxValue, vuRectangle);
    dc.rectangle(settings.padding, y, settings.tickWidth, 1);
    dc.set_source(color);
    dc.fill();

    double tickDb = settings.tickDb;

    Lv2cColor minorTickColor = Lv2cColor(color, 0.60);
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
        y = Lv2cVuElement::ValueToClient(db,minValue,maxValue, vuRectangle);
        dc.rectangle(settings.padding, y, settings.tickWidth, 1);
        dc.fill();
    }
    for (double db = -tickDb; db > minValue; db -= tickDb)
    {
        y = Lv2cVuElement::ValueToClient(db,minValue,maxValue, vuRectangle);
        dc.rectangle(settings.padding, y, settings.tickWidth, 1);
        dc.fill();
    }
}

/* static*/
void Lv2cDbVuElement::DrawTelltale(
    Lv2cDrawingContext &dc,
    double telltaleValue,
    double value,
    double minValue,
    double maxValue,
    const Lv2cVuSettings &settings,
    const Lv2cRectangle &vuBarRectangle)
{
    double y = Lv2cVuElement::ValueToClient(telltaleValue, minValue, maxValue, vuBarRectangle);
    double yValue = Lv2cVuElement::ValueToClient(value, minValue, maxValue, vuBarRectangle);
    if (std::abs(y - yValue) > 1)
    {
        double height = 1;
        Lv2cColor color = settings.green;
        if (settings.yellowLevel.has_value())
        {
            double yYellow = Lv2cVuElement::ValueToClient(settings.yellowLevel.value(), minValue, maxValue, vuBarRectangle);
            if (y < yYellow)
            {
                color = settings.yellow;
            }
        }
        if (settings.redLevel.has_value())
        {
            double yRed = Lv2cVuElement::ValueToClient(settings.redLevel.value(), minValue, maxValue, vuBarRectangle);
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

void Lv2cDbVuElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc); // draws the vus,

    Lv2cRectangle clientRectangle{ClientSize()};

    auto &settings = Settings();
    Lv2cRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
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
        Lv2cRectangle vuBarRectangle = Lv2cRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());

        DrawTelltale(dc,HoldValue(),Value(),MinValue(),MaxValue(), Settings(), vuBarRectangle);
    }
}

void Lv2cStereoDbVuElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc); // draws the vus,

    Lv2cRectangle clientRectangle{ClientSize()};

    auto &settings = Settings();
    Lv2cRectangle vuRectangle = clientRectangle.Inflate(-settings.padding);
    Lv2cDbVuElement::DrawTicks(
        dc,
        MinValue(),MaxValue(),
        Settings(),
        vuRectangle,
        Theme().vuTickColor);

    // draw the telltale
    double offsetX = settings.tickWidth + settings.padding;
    Lv2cRectangle vuBoxRectangle = Lv2cRectangle(vuRectangle.Left() + offsetX, vuRectangle.Top(), vuRectangle.Width() - offsetX, vuRectangle.Height());
    double vuWidth = (vuBoxRectangle.Width()-settings.padding)/2;
    if (HoldValue() != Value())
    {
        Lv2cRectangle vuLeft { vuBoxRectangle.Left(),vuBoxRectangle.Top(),vuWidth,vuBoxRectangle.Height()};
        Lv2cDbVuElement::DrawTelltale(dc,HoldValue(),Value(),MinValue(),MaxValue(), Settings(), vuLeft);
    }
    if (RightHoldValue() != RightValue())
    {
        Lv2cRectangle vuRight { vuBoxRectangle.Right()-vuWidth,vuBoxRectangle.Top(),vuWidth,vuBoxRectangle.Height()};
        Lv2cDbVuElement::DrawTelltale(dc,RightHoldValue(),RightValue(),MinValue(),MaxValue(), Settings(), vuRight);
    }
}


Lv2cDbVuElement::Lv2cDbVuElement()
{
    HoldValueProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}
Lv2cStereoDbVuElement::Lv2cStereoDbVuElement()
{
    leftAnimationStartTime = rightAnimationStartTime = clock_t::now();

    HoldValueProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
    RightHoldValueProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}

void Lv2cDbVuElement::OnMount()
{
    super::OnMount();
    HoldValue(Value());
    StopAnimation();
}

void Lv2cStereoDbVuElement::OnMount()
{
    super::OnMount();
    HoldValue(Value());
    RightHoldValue(RightValue());
    StopAnimation();
}

void Lv2cDbVuElement::OnUnmount()
{
    StopAnimation();
}
void Lv2cStereoDbVuElement::OnUnmount()
{
    StopAnimation();
}
void Lv2cDbVuElement::OnValueChanged(double value)
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
void Lv2cStereoDbVuElement::OnValueChanged(double value)
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
void Lv2cStereoDbVuElement::OnRightValueChanged(double value)
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


void Lv2cDbVuElement::StopAnimation()
{
    if (animationHandle)
    {
        this->Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
}
void Lv2cStereoDbVuElement::StopAnimation()
{
    if (animationHandle)
    {
        this->Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
    }
    rightAnimationActive = false;
    leftAnimationActive = false;
}

void Lv2cDbVuElement::AnimationTick(const animation_clock_time_point_t&now)
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
void Lv2cStereoDbVuElement::AnimationTick(const animation_clock_time_point_t&now)
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

void Lv2cDbVuElement::StartAnimation(bool hold)
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
void Lv2cStereoDbVuElement::StartLeftAnimation(bool hold)
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

void Lv2cStereoDbVuElement::StartRightAnimation(bool hold)
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

double Lv2cDbVuElement::ValueToClient(double value, const Lv2cRectangle &vuRectangle)
{
    return Lv2cVuElement::ValueToClient(value, MinValue(), MaxValue(), vuRectangle);
}
