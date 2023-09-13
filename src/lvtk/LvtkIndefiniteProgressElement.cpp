#include "lvtk/LvtkIndefiniteProgressElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkDrawingContext.hpp"

using namespace lvtk;

LvtkIndefiniteProgressElement::LvtkIndefiniteProgressElement()
{
    ActiveProperty.SetElement(this, &LvtkIndefiniteProgressElement::OnActiveChanged);
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
void LvtkIndefiniteProgressElement::OnMount()
{
    super::OnMount();
    ClearClasses();
    if (Active())
    {
        StartAnimation();
    }
    AddClass(Theme().indefiniteProgressStyle);
}
void LvtkIndefiniteProgressElement::OnUnmount()
{
    StopAnimation();
    super::OnUnmount();
}

bool LvtkIndefiniteProgressElement::WillDraw() const
{
    if (Active())
    {
        return true;
    }
    return super::WillDraw();
}

void LvtkIndefiniteProgressElement::StartAnimation()
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

void LvtkIndefiniteProgressElement::StopAnimation()
{
    animator.CancelAnimation();
}

void LvtkIndefiniteProgressElement::OnActiveChanged(bool value)
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

void LvtkIndefiniteProgressElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    LvtkRectangle clientRect(ClientSize());

    if (Active())
    {
        LvtkColor color = Style().Color().get_color();
        LvtkColor transparent{color, 0.1};
        std::vector<LvtkColorStop> colorStops;
        colorStops.reserve(8);

        // if (animationValue >= 0.5)
        // {
        //     double otherValue = animationValue-1;
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue-0.1),transparent));
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue-0.05),color));
        //     colorStops.push_back(LvtkColorStop(
        //         otherValue,color));
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue+0.02),transparent));
        // }
        double animationValue = this->animationValue/this->ClientSize().Width();
        colorStops.push_back(LvtkColorStop(
            (animationValue - beadRatio*4), transparent));
        colorStops.push_back(LvtkColorStop(
            (animationValue - beadRatio), color));
        colorStops.push_back(LvtkColorStop(
            animationValue, color));
        colorStops.push_back(LvtkColorStop(
            (animationValue + beadRatio/4), transparent));

        // if (animationValue < 0.5)
        // {
        //     double otherValue = animationValue+1;
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue-0.1),transparent));
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue-0.05),color));
        //     colorStops.push_back(LvtkColorStop(
        //         otherValue,color));
        //     colorStops.push_back(LvtkColorStop(
        //         (otherValue+0.02),transparent));
        // }

        dc.set_source(LvtkPattern::linear_gradient(0, 0, clientRect.Width(), 0, colorStops));
        dc.rectangle(clientRect);
        dc.fill();
    }
}

void LvtkIndefiniteProgressElement::OnLayoutComplete()
{
    UpdateAnimationForLayout();
}

void LvtkIndefiniteProgressElement::UpdateAnimationForLayout()
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
