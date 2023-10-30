// Copyright (c) 2023 Robin E. R. Davies
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

#include "TunerTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c_ui/Lv2TunerElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include <chrono>

#include "lv2c/Lv2cVerticalStackElement.hpp"

using namespace lv2c;
using namespace lv2c::ui;



class TunerTestElement : public Lv2cContainerElement {
public:
    using clock_t = std::chrono::steady_clock;

    using self = TunerTestElement;
    using super = Lv2cContainerElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }

    TunerTestElement()
    {
        tuner = Lv2TunerElement::Create();
        AddChild(tuner);
        tunerFrequency.Bind(tuner->ValueProperty);

    }
protected:
    virtual void OnMount() {
        clockStartTime = clock_t::now();

        animationValue = 0;
        tickHandle = Window()->PostDelayed(1000/30,
            [this]() {
                AnimationTick();
            }
        );
    }
    virtual void OnUnmount() {
        if (tickHandle)
        {
            Window()->CancelPostDelayed(tickHandle);
            tickHandle = AnimationHandle::InvalidHandle;
        }
    }

    void AnimationTick()
    {
        clock_t::time_point now = clock_t::now();

        using fsec = std::chrono::duration<double>;
        
        double seconds = std::chrono::duration_cast<fsec>(now-clockStartTime).count();
        seconds = std::fmod(seconds,12*2+3);
        if (seconds < 3) {
            tunerFrequency.set(0);
        } else {
            double frequency = 440.0*std::pow(2,(seconds*0.5-0.5)/12.0);
            tunerFrequency.set(frequency);
        }

        tickHandle = Window()->PostDelayed(1000/30,
            [this]() {
                AnimationTick();
            }
        );
    }

private:
    clock_t::time_point clockStartTime;
    double animationValue = 0;
    AnimationHandle tickHandle = AnimationHandle::InvalidHandle;
    Lv2cBindingProperty<double> tunerFrequency;

    Lv2TunerElement::ptr tuner;
};

Lv2cElement::ptr TunerTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cFlexGridElement::ptr main = Lv2cFlexGridElement::Create();
    main->Style().FlexWrap(Lv2cFlexWrap::Wrap);
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16});
    {
        auto element = TunerTestElement::Create();
        main->AddChild(element);
    }

    return main;
}
