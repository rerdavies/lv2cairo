/*
Copyright (c) 2023 Robin E. R. Davies

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "lv2c_ui/Lv2TunerElement.hpp"
#include "lv2c_ui/Lv2ControlConstants.hpp"
#include "lv2c/Lv2cDrawingContext.hpp"

#include "pango/pangocairo.h"
#include "lv2c/Lv2cPangoContext.hpp"

using namespace lvtk;
using namespace lvtk::ui;

Lv2TunerElement::Lv2TunerElement()
{
    double width = 200;
    double height = Lv2ControlHeight - 16;

    auto backgroundGradient = Lv2cPattern::linear_gradient(
        0, height,
        0, 0,
        {
            Lv2cColorStop{0.0, Lv2cColor("#A0A880")},
            Lv2cColorStop{0.95, Lv2cColor("#908870")},
            Lv2cColorStop{1.0, Lv2cColor("#504438")},
        });
    Style()
        .RoundCorners({8})
        .Background(
            backgroundGradient)
        .Width(width)
        .Height(height)
        .Color(Lv2cColor{"#404040"});
}
void Lv2TunerElement::OnValueChanged(double value)
{
    Invalidate();
}

void Lv2TunerElement::OnMount()
{
    Style()
        .FontFamily(Theme().bodyPrimaryStyle->FontFamily())
        .FontSize(Lv2cMeasurement::Point(24))
        .FontWeight(Lv2cFontWeight::Normal);
}

void Lv2TunerElement::DrawDial(Lv2cDrawingContext &dc, double midiNote)
{
    double cents;
    if (midiNote < 0)
    {
        cents = -0.50;
    }
    else
    {
        int iNote = (int)std::round(midiNote);
        cents = midiNote - iNote;
    }

    Lv2cRectangle clientSize = this->ClientSize();
    double radius = clientSize.Width()*1;
    double maxAngle = std::tan((clientSize.Width()*0.45)/radius);
    double dialScale = maxAngle/0.30;


    dc.save();
    {
        dc.translate(clientSize.Width()/2,radius+8);
        static std::vector<double> dialTicks { -0.30,-0.20,-0.10,-0.05,-0.03,-0.02,-0.01,0,0.01,0.02,0.03,0.05,0.10,0.20,0.30};

        dc.set_source(Style().Color());
        for (auto tick: dialTicks)
        {
            dc.save();
            {
                double innerRadius = -radius+18;
                if (abs(tick) < 0.10 && tick != 0)
                {
                    innerRadius = -radius+12;
                }
                dc.rotate(dialScale*tick);
                dc.move_to(-1,-radius+3);
                dc.line_to(1,-radius+3);
                dc.line_to(1,innerRadius);
                dc.line_to(-1,innerRadius);
                dc.close_path();
                dc.fill();
            }
            dc.restore();
        }

        dc.save();
        {
            if (cents < -0.30) cents = -0.30;
            if (cents > 0.30) cents = 0.30;
            double needleAngle = cents*dialScale;
            dc.rotate(needleAngle);
            dc.move_to(-3,0);
            dc.line_to(-1,-radius);
            dc.line_to(1,-radius);
            dc.line_to(3,0);
            dc.close_path();
            dc.set_source(Lv2cColor("#800000"));
            dc.fill();
        }
        dc.restore();
    }
    dc.restore();



}

void Lv2TunerElement::DrawText(Lv2cDrawingContext &dc, double midiNote)
{

    std::string noteName;
    std::string centsText;
    if (midiNote < 0)
    {
        noteName = "−−";
        centsText = "−−";
    }
    else
    {
        int iNote = (int)std::round(midiNote);
        double cents = midiNote-iNote;
        int octave = iNote / 12;
        int pitch = iNote - octave * 12;

        std::stringstream s;
        static const char *PITCH_NAMES[] = {
            "C", "C♯", "D", "E♭", "E", "F", "F♯", "G", "A♭", "A", "B♭", "B"};
        s << PITCH_NAMES[pitch] << (octave-1);
        noteName = s.str();

        std::stringstream sc;
        int iCents = (int)std::round(cents * 100);
        if (iCents < 0)
        {
            sc << "−.";
            iCents = -iCents;
        }
        else
        {
            sc << "+.";
        }
        sc << (char)('0' + (iCents / 10 % 10));
        sc << (char)('0' + (iCents % 10));
        centsText = sc.str();
    }
    dc.set_source(Style().Color());
    pango_layout_set_text(pangoLayout, noteName.c_str(), (int)(noteName.length()));
    PangoRectangle inkRect, logicalRect;
    pango_layout_get_extents(pangoLayout, &inkRect, &logicalRect);
    Lv2cSize pangoSize = Lv2cSize(std::ceil(logicalRect.width / PANGO_SCALE), std::ceil(logicalRect.height / PANGO_SCALE));

    double center = std::floor(clientSize.Width() / 2);
    constexpr double TEXT_SPACE = 16;
    Lv2cPoint ptText = dc.round_to_device(
        Lv2cPoint(
            32,
            clientSize.Height() - pangoSize.Height()));

    dc.move_to(ptText.x, ptText.y);
    pango_cairo_show_layout(dc.get(), pangoLayout);

    pango_layout_set_text(pangoLayout, centsText.c_str(), (int)(centsText.length()));
    pango_layout_get_extents(pangoLayout, &inkRect, &logicalRect);
    pangoSize = Lv2cSize(std::ceil(logicalRect.width / PANGO_SCALE), std::ceil(logicalRect.height / PANGO_SCALE));
    ptText = dc.round_to_device(Lv2cPoint(
        center + TEXT_SPACE,
        clientSize.Height() - pangoSize.Height()));

    dc.move_to(ptText.x, ptText.y);
    pango_cairo_show_layout(dc.get(), pangoLayout);
}
void Lv2TunerElement::OnDraw(Lv2cDrawingContext &dc)
{
    PreparePangoContext();
    Lv2cRectangle rcClient{ClientSize()};

    auto roundCorners = Style().RoundCorners().PixelValue();
    {


        dc.set_source(Style().Background());
        dc.round_corner_rectangle(rcClient,roundCorners);
        dc.fill();


        double midiNote;
        if (this->ValueIsMidiNote())
        {
            midiNote = Value();
        } else {

            double frequency = Value();
            if (frequency <= 0)
            {
                midiNote = -1;
            } {
                midiNote = std::log2(frequency/ReferenceFrequency())*12 + 69;
            }
        }

        DrawText(dc,midiNote);
        DrawDial(dc,midiNote);
    }
}

Lv2TunerElement::~Lv2TunerElement()
{
    FreePangoContext();
}
void Lv2TunerElement::PreparePangoContext()
{
    if (pangoLayout == nullptr)
    {

        // pangoLayout = pango_cairo_create_layout(context.get());

        pangoLayout = pango_layout_new(GetPangoContext());
        auto fontDescriptor = gPangoContext.GetFontDescription(this->Style());
        pango_layout_set_font_description(pangoLayout, fontDescriptor);
        pango_font_description_free(fontDescriptor);
    }
}

void Lv2TunerElement::FreePangoContext()
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
        pangoLayout = nullptr;
    }
}
