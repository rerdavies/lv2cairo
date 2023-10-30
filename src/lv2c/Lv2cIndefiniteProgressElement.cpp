#include "lv2c/Lv2cIndefiniteProgressElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cDrawingContext.hpp"

using namespace lv2c;

Lv2cIndefiniteProgressElement::Lv2cIndefiniteProgressElement()
{
    ActiveProperty.SetElement(this, &Lv2cIndefiniteProgressElement::OnActiveChanged);
    using namespace std::chrono;
    animator.Initialize(
        this, 1500ms, 15000ms,
        [this](double value)
        {
            this->animationValue = value;
            double width = ClientSize().Width();
            if (value >= width)
            {
                // loop indefinitely
                animator.Animate(
                    0, width,
                    width/RATE
                    );
            }
            Invalidate();
        });
}
void Lv2cIndefiniteProgressElement::OnMount()
{
    super::OnMount();
    ClearClasses();
    if (Active())
    {
        StartAnimation();
    }
    AddClass(Theme().indefiniteProgressStyle);
}
void Lv2cIndefiniteProgressElement::OnUnmount()
{
    StopAnimation();
    super::OnUnmount();
}

bool Lv2cIndefiniteProgressElement::WillDraw() const
{
    if (Active())
    {
        return true;
    }
    return super::WillDraw();
}

void Lv2cIndefiniteProgressElement::StartAnimation()
{
    if (IsMounted())
    {
        double width = ClientSize().Width();
        if (width != 0)
        {
            animator.Animate(0, width,width/RATE);
        } else 
        {
            // OnLayoutComplete will fix it.
        }
    }

}

void Lv2cIndefiniteProgressElement::StopAnimation()
{
    animator.CancelAnimation();
}

void Lv2cIndefiniteProgressElement::OnActiveChanged(bool value)
{
    if (IsMounted())
    {
        if (Active())
        {
            StartAnimation(); 
        }
        else
        {
            StopAnimation();
            Invalidate();
        }
    }
}

void Lv2cIndefiniteProgressElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    Lv2cRectangle clientRect(ClientSize());

    if (Active())
    {
        Lv2cColor color = Style().Color().get_color();
        Lv2cColor transparent{color, 0.1};
        std::vector<Lv2cColorStop> colorStops;
        colorStops.reserve(8);

        // if (animationValue >= 0.5)
        // {
        //     double otherValue = animationValue-1;
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue-0.1),transparent));
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue-0.05),color));
        //     colorStops.push_back(Lv2cColorStop(
        //         otherValue,color));
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue+0.02),transparent));
        // }
        double animationValue = this->animationValue/this->ClientSize().Width();
        colorStops.push_back(Lv2cColorStop(
            (animationValue - beadRatio*4), transparent));
        colorStops.push_back(Lv2cColorStop(
            (animationValue - beadRatio), color));
        colorStops.push_back(Lv2cColorStop(
            animationValue, color));
        colorStops.push_back(Lv2cColorStop(
            (animationValue + beadRatio/4), transparent));

        // if (animationValue < 0.5)
        // {
        //     double otherValue = animationValue+1;
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue-0.1),transparent));
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue-0.05),color));
        //     colorStops.push_back(Lv2cColorStop(
        //         otherValue,color));
        //     colorStops.push_back(Lv2cColorStop(
        //         (otherValue+0.02),transparent));
        // }

        dc.set_source(Lv2cPattern::linear_gradient(0, 0, clientRect.Width(), 0, colorStops));
        dc.rectangle(clientRect);
        dc.fill();
    }
}

void Lv2cIndefiniteProgressElement::OnLayoutComplete()
{
    UpdateAnimationForLayout();
}

void Lv2cIndefiniteProgressElement::UpdateAnimationForLayout()
{
    using namespace std::chrono;

    double width = this->ClientSize().Width();

    if (this->Active())
    {
        double currentValue = animator.Value();
        if (currentValue >= width)
        {
            animator.Animate(
                0, width,
                (width / RATE));
        }
        else
        {
            animator.Animate(
                currentValue,
                width,
                (width-currentValue) / RATE);
        }
        this->beadRatio = 10/width;
    }
    else
    {
        animator.CancelAnimation();
    }
}
