#include "lv2c/Lv2cLampElement.hpp"
#include "lv2c/Lv2cPngElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"
#include "lv2c/Lv2cWindow.hpp"

using namespace lvtk;

Lv2cLampElement::Lv2cLampElement()
{
    // VariantProperty.SetElement(this,&Lv2cLampElement::OnVariantChanged);
    // auto bg = Lv2cPngElement::Create();
    // bg->Source("OffLed3.png");
    // bg->Style()
    //     .HorizontalAlignment(Lv2cAlignment::Stretch)
    //     .VerticalAlignment(Lv2cAlignment::Stretch);
    // AddChild(bg);

    // auto image = Lv2cPngElement::Create();
    // image->Source("BlueLed3.png");
    // image->Style()
    //     .HorizontalAlignment(Lv2cAlignment::Stretch)
    //     .VerticalAlignment(Lv2cAlignment::Stretch);
    // AddChild(image);
    // this->image = image;
    // image->Style().Opacity(Value());
    this->image = Lv2cElement::Create();
    image->Style()
        .RoundCorners({4})
        .Height(18)
        .Width(18)
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center)
        //.BorderWidth({1})
        .BorderColor(Lv2cColor(0, 0, 0));

    auto dropShadowSetting = Lv2cDropShadow{
        .variant = Lv2cDropShadowVariant::InnerDropShadow,
        .xOffset = 0.1,
        .yOffset = 2.0,
        .radius = 6.0,
        .opacity = 0.95,
    };
    auto dropShadow = Lv2cDropShadowElement::Create();
    dropShadow->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);

    dropShadow->DropShadow(dropShadowSetting);
    AddChild(dropShadow);

    dropShadow->AddChild(image);

    OnValueChanged(Value());
}

void Lv2cLampElement::OnVariantChanged(Lv2cLampVariant value)
{
    OnValueChanged(Value());
}

void Lv2cLampElement::OnValueChanged(double value)
{
    double adjustedValue;
    if (Variant() == Lv2cLampVariant::OnOff)
    {
        adjustedValue = value >= 0.5 ? 1.0 : 0.0;
    }
    else
    {
        adjustedValue = 1 - (1 - value) * (1 - value);
    }
    StartAnimation(adjustedValue);
}

void Lv2cLampElement::AnimationTick(const animation_clock_time_point_t&now)
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

double Lv2cLampElement::AnimationValue() const { return animationValue; }
void Lv2cLampElement::StartAnimation(double targetValue)
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
void Lv2cLampElement::StopAnimation()
{
    if (animationHandle)
    {
        Window()->CancelAnimationCallback(animationHandle);
        animationHandle = AnimationHandle::InvalidHandle;
        AnimationValue(animationTarget);
    }
}
void Lv2cLampElement::AnimationValue(double value)
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

void Lv2cLampElement::UpdateLampColor()
{
        Lv2cColor color = Lv2cColor::Blend(animationValue, Theme().lampOffColor, Theme().lampOnColor);
        image->Style().Background(color);
        Invalidate();

}

void Lv2cLampElement::OnUnmount()
{
    StopAnimation();
}

void Lv2cLampElement::OnMount()
{
    Classes({Theme().lampStyle});
    animationValue = Value();
    UpdateLampColor();
}
void Lv2cLampElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    // double value = Value();
    // Lv2cRectangle clientRectangle {this->ClientSize()};
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
