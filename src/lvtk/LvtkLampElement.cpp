#include "lvtk/LvtkLampElement.hpp"
#include "lvtk/LvtkPngElement.hpp"
#include "lvtk/LvtkDropShadowElement.hpp"
#include "lvtk/LvtkWindow.hpp"

using namespace lvtk;

LvtkLampElement::LvtkLampElement()
{
    // VariantProperty.SetElement(this,&LvtkLampElement::OnVariantChanged);
    // auto bg = LvtkPngElement::Create();
    // bg->Source("OffLed3.png");
    // bg->Style()
    //     .HorizontalAlignment(LvtkAlignment::Stretch)
    //     .VerticalAlignment(LvtkAlignment::Stretch);
    // AddChild(bg);

    // auto image = LvtkPngElement::Create();
    // image->Source("BlueLed3.png");
    // image->Style()
    //     .HorizontalAlignment(LvtkAlignment::Stretch)
    //     .VerticalAlignment(LvtkAlignment::Stretch);
    // AddChild(image);
    // this->image = image;
    // image->Style().Opacity(Value());
    this->image = LvtkElement::Create();
    image->Style()
        .RoundCorners({4})
        .Height(18)
        .Width(18)
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center)
        //.BorderWidth({1})
        .BorderColor(LvtkColor(0, 0, 0));

    auto dropShadowSetting = LvtkDropShadow{
        .variant = LvtkDropShadowVariant::InnerDropShadow,
        .xOffset = 0.1,
        .yOffset = 2.0,
        .radius = 6.0,
        .opacity = 0.95,
    };
    auto dropShadow = LvtkDropShadowElement::Create();
    dropShadow->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);

    dropShadow->DropShadow(dropShadowSetting);
    AddChild(dropShadow);

    dropShadow->AddChild(image);

    OnValueChanged(Value());
}

void LvtkLampElement::OnVariantChanged(LvtkLampVariant value)
{
    OnValueChanged(Value());
}

void LvtkLampElement::OnValueChanged(double value)
{
    double adjustedValue;
    if (Variant() == LvtkLampVariant::OnOff)
    {
        adjustedValue = value >= 0.5 ? 1.0 : 0.0;
    }
    else
    {
        adjustedValue = 1 - (1 - value) * (1 - value);
    }
    StartAnimation(adjustedValue);
}

void LvtkLampElement::AnimationTick(const animation_clock_time_point_t&now)
{
    using namespace std::chrono;
    this->animationHandle = AnimationHandle::InvalidHandle;

    constexpr clock_t::rep TICKS_PER_SECOND = duration_cast<clock_t::duration>(1000ms).count();
    constexpr double TICKS_TO_SECONDS = 1.0 / TICKS_PER_SECOND;

    auto dt = clock_t::now() - animationStartTime;

    double seconds = dt.count() * TICKS_TO_SECONDS;

    bool animating = true;
    if (animationTarget < AnimationValue())
    {
        const double DECREMENT_RATE = 1 / 0.5;
        double newValue = animationStartValue - seconds * DECREMENT_RATE;
        // std::cout << "Tick: " << seconds << " start: " << animationStartValue << " new: " << newValue << std::endl;
        if (newValue < 0)
        {
            AnimationValue(0);
            animating = false;
        }
        else
        {
            AnimationValue(newValue);
        }
    }
    else
    {
        AnimationValue(animationTarget);
        animating = false;
    }
    if (animating)
    {
        Window()->RequestAnimationCallback(
            [this](const animation_clock_time_point_t &now)
            {
                AnimationTick(now);
            });
    }
}

double LvtkLampElement::AnimationValue() const { return animationValue; }
void LvtkLampElement::StartAnimation(double targetValue)
{
    if (targetValue == animationTarget && animationHandle)
    {
        return;
    }
    if (Window())
    {
        this->animationTarget = targetValue;
        this->animationStartTime = clock_t::now();
        this->animationStartValue = AnimationValue();
        if (!animationHandle)
        {
            animationHandle = Window()->RequestAnimationCallback(
                [this](const animation_clock_time_point_t &now)
                { AnimationTick(now); });
        }
    }
    else
    {
        AnimationValue(targetValue);
    }
}
void LvtkLampElement::StopAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
        AnimationValue(animationTarget);
    }
}
void LvtkLampElement::AnimationValue(double value)
{
    if (!IsMounted())
    {
        return;
    }
    if (value != animationValue)
    {
        animationValue = value;
        UpdateLampColor();
    }
}

void LvtkLampElement::UpdateLampColor()
{
        LvtkColor color = LvtkColor::Blend(animationValue, Theme().lampOffColor, Theme().lampOnColor);
        image->Style().Background(color);
        Invalidate();

}

void LvtkLampElement::OnUnmount()
{
    StopAnimation();
}

void LvtkLampElement::OnMount()
{
    Classes({Theme().lampStyle});
    animationValue = Value();
    UpdateLampColor();
}
void LvtkLampElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    // double value = Value();
    // LvtkRectangle clientRectangle {this->ClientSize()};
    // auto corners = Style().RoundCorners().PixelValue();
    // auto borderWidth = Style().BorderWidth().PixelValue();

    // corners = corners.inset(borderWidth);

    // dc.save();
    // {
    //     dc.round_corner_rectangle(clientRectangle,corners);
    //     dc.clip();
    //     dc.set_source(Style().Color());
    //     dc.paint_with_alpha(value);
    // }
    // dc.restore();
}
