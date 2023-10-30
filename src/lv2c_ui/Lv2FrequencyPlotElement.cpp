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

#include "lv2c_ui/Lv2FrequencyPlotElement.hpp"
#include "lv2c_ui/Lv2UI.hpp"
#include "lv2c/Lv2cDrawingContext.hpp"
#include <lv2/atom/atom.h>
#include <cmath>

using namespace lv2c::ui;
using namespace lv2c;

static constexpr float MIN_DB = -200;
static constexpr float MIN_DB_AMPLITUDE = 1e-10f;

static inline float Af2Db(float value)
{
    if (value < MIN_DB_AMPLITUDE)
        return MIN_DB;
    return 20.0f * std::log10(value);
}

// static float DB2A_FACTOR = std::log(10.0f) * 0.05f;

// static inline float Db2Af(float value)
// {
//     if (value < MIN_DB)
//         return 0;
//     return std::exp(value * (DB2A_FACTOR));
// }

Lv2FrequencyPlotElement::Lv2FrequencyPlotElement(Lv2UI *lv2UI, const UiFrequencyPlot *frequencyPlot)
    : lv2UI(lv2UI), frequencyPlot(*frequencyPlot)
{
    InitUrids();

    // precompute log(f) y grid coordinates.
    PreComputeGridXs();
}
void Lv2FrequencyPlotElement::PreComputeGridXs()
{
    majorGridXs.resize(0);
    minorGridXs.resize(0);
    
    double m = frequencyPlot.width() / (std::log(frequencyPlot.xRight()) - std::log(frequencyPlot.xLeft()));

    double gridX0 = std::pow(10, std::floor(std::log10(this->frequencyPlot.xLeft())));
    for (double f = gridX0; f < this->frequencyPlot.xRight(); f *= 10)
    {
        for (size_t i = 1; i <= 9; ++i)
        {
            double decade = f * i;
            if (decade >= this->frequencyPlot.xLeft() && decade < this->frequencyPlot.xRight())
            {
                double x = m * (std::log(decade) - std::log(frequencyPlot.xLeft()));
                if (x > 0 && x < frequencyPlot.width())
                {
                    if (i == 1)
                    {
                        majorGridXs.push_back(x);
                    }
                    else
                    {
                        minorGridXs.push_back(x);
                    }
                }
            }
        };
    }
}

void Lv2FrequencyPlotElement::InitUrids()
{
    urids.propertyUrid = lv2UI->GetUrid(this->frequencyPlot.patchProperty().c_str());
    urids.atom__Float = lv2UI->GetUrid(LV2_ATOM__Float);
    urids.atom__Vector = lv2UI->GetUrid(LV2_ATOM__Vector);
}
bool Lv2FrequencyPlotElement::WillDraw() const
{
    return true;
}
void Lv2FrequencyPlotElement::OnMount()
{
    this->ClearClasses();
    super::OnMount();
    this->AddClass(Theme().plotStyle);

    this->Style()
        .Width(frequencyPlot.width());

    lv2UI->RequestPatchProperty(this->urids.propertyUrid);
    propertyEventHandle = lv2UI->OnPatchProperty.AddListener([this](const Lv2UI::PatchPropertyEventArgs &e)
                                                             {
        if (e.property == this->urids.propertyUrid)
        {
            OnValuesChanged(e.value);
        }
        return false; });
}
void Lv2FrequencyPlotElement::OnUnmount()
{
    lv2UI->OnPatchProperty.RemoveListener(propertyEventHandle);
    super::OnUnmount();
}

void Lv2FrequencyPlotElement::OnValuesChanged(const void *data)
{
    const LV2_Atom_Vector *atomVector = (const LV2_Atom_Vector *)data;
    if (atomVector->atom.type == urids.atom__Vector && atomVector->body.child_type == urids.atom__Float)
    {
        size_t count = (atomVector->atom.size - sizeof(LV2_Atom_Vector_Body)) / atomVector->body.child_size;
        const float *newValues = (const float *)((const uint8_t *)data + sizeof(LV2_Atom_Vector));

        bool axesChanged = true;
        if (count == this->values.size() + 4)
        {
            bool changed = false;
            axesChanged = false;
            axesChanged 
                = frequencyPlot.xLeft() != newValues[0]
                || frequencyPlot.xRight() != newValues[1]
                || frequencyPlot.yTop() != newValues[2]
                || frequencyPlot.yBottom() != newValues[3];

            for (size_t i = 0; i < values.size(); ++i)
            {
                if (this->values[i] != newValues[i+4])
                {
                    changed = true;
                    break;
                }
            }
            if (!changed && !axesChanged)
            {
                return;
            }
        }

        if (axesChanged)
        {
            frequencyPlot.xLeft(newValues[0]);
            frequencyPlot.xRight(newValues[1]);
            frequencyPlot.yTop(newValues[2]);
            frequencyPlot.yBottom(newValues[3]);
            PreComputeGridXs();
        } 
        this->values.resize(count-4);
        for (size_t i = 0; i < this->values.size(); ++i)
        {
            this->values[i] = newValues[i+4];
        }
        Invalidate();
    }
}

void Lv2FrequencyPlotElement::DrawTicks(Lv2cDrawingContext &dc)
{
    Lv2cSize clientSize = this->ClientSize();
    constexpr double minorTickWidth = 0.20;
    constexpr double majorTickWidth = 0.35;
    dc.set_source(Theme().plotTickColor);
    dc.set_line_width(minorTickWidth);
    dc.set_line_cap(cairo_line_cap_t::CAIRO_LINE_CAP_BUTT);
    for (double x : minorGridXs)
    {
        dc.move_to(x, 0);
        dc.line_to(x, clientSize.Height());
        dc.stroke();
    }
    dc.set_line_width(majorTickWidth);
    double gxScale = clientSize.Width()/this->frequencyPlot.width();
    for (double gx : majorGridXs)
    {
        double x = gx*gxScale;
        dc.move_to(x, 0);
        dc.line_to(x, clientSize.Height());
        dc.stroke();
    }

    {
        // solve for m, c:
        // f(x) = m*x+c;
        // f(yBottom) = height()
        // f(yTop) = 0
        //  m*yTop + c =0
        //  m*yBottom+c = height()
        // m*(yBottom-yTop) = height()

        dc.set_line_width(0.35);

        double m = clientSize.Height() / (frequencyPlot.yBottom() - frequencyPlot.yTop());
        double c = -m * frequencyPlot.yTop();
        for (
            double db = std::floor(frequencyPlot.yBottom() / 10) * 10 + 10;
            db < frequencyPlot.yTop();
            db += 10)
        {
            double y = m * db + c;
            dc.set_line_width(db == 0 ? majorTickWidth : minorTickWidth);
            dc.move_to(0, y);
            dc.line_to(frequencyPlot.width(), y);
            dc.stroke();
        }
    }
}

void Lv2FrequencyPlotElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    auto clientSize = ClientSize();
    Lv2cRectangle clientRect{clientSize};

    dc.save();
    {
        Lv2cRoundCorners corners = Style().RoundCorners().PixelValue();
        dc.round_corner_rectangle(clientRect, corners);
        dc.clip();

        DrawTicks(dc);
        size_t count = this->values.size();
        auto clientSize = ClientSize();
        if (count > 1)
        {
            double dx = clientSize.Width() / (count - 1);
            // y = m*x+c;
            // f(MAX_Y) = 0;
            // f(MIN_Y) = frequencyPlot.width()
            double m = clientSize.Height() / (frequencyPlot.yBottom() - frequencyPlot.yTop());
            double c = -frequencyPlot.yTop() * m;

            for (size_t i = 0; i < count; ++i)
            {
                double x = dx * i;
                double y = values[i];
                y = Af2Db(y);
                y = m * y + c;
                if (i == 0)
                {
                    dc.move_to(x-1, y);
                    
                }
                dc.line_to(x, y);
            }
            dc.set_line_cap(cairo_line_cap_t::CAIRO_LINE_CAP_ROUND);
            dc.set_line_width(3);

            dc.set_source(Theme().plotColor);
            dc.stroke();
        }
    }
    dc.restore();
}