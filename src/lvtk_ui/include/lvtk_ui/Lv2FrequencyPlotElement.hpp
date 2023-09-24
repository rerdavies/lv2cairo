// Copyright (c) 2023 Robin Davies
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

#pragma once
#include "lvtk/LvtkElement.hpp"
#include "lvtk_ui/PiPedalUI.hpp"
#include "lvtk_ui/Lv2UI.hpp"

namespace lvtk::ui {

    class Lv2FrequencyPlotElement: public LvtkElement {
    public:
        using self=Lv2FrequencyPlotElement;
        using super=LvtkElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2UI*lv2UI,const UiFrequencyPlot*frequencyPlot) {
            return std::make_shared<self>(lv2UI,frequencyPlot);
        }
        Lv2FrequencyPlotElement(Lv2UI*lv2UI,const UiFrequencyPlot*frequencyPlot);
    protected:
        virtual bool WillDraw() const override;
        virtual void OnMount() override;
        virtual void OnUnmount() override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
    private:
        void PreComputeGridXs();
        void DrawTicks(LvtkDrawingContext &dc);
        EventHandle propertyEventHandle;
        struct Urids {
            LV2_URID propertyUrid;
            LV2_URID atom__Vector;
            LV2_URID atom__Float;
        };
        Urids urids;
        void InitUrids();

        void OnValuesChanged(const void*value);
        Lv2UI*lv2UI = nullptr;
        const UiFrequencyPlot frequencyPlot;
        std::vector<float> values;
        std::vector<double> majorGridXs;
        std::vector<double> minorGridXs;
    };
}