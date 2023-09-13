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

#include "SamplePlugin.hpp"
#include <stdexcept>
#include <numbers>
#include <cmath>


static PluginRegistration<SamplePlugin> registration(PLUGIN_URI);


SamplePlugin::SamplePlugin(
    double rate,
    const char *bundle_path,
    const LV2_Feature *const *features)
    :super(rate,bundle_path,features)
{
}

void SamplePlugin::ConnectPort(uint32_t port, void *data)
{
    switch ((PortId)port)
    {
    case PortId::Level:
        level = (const float*)data;
        break;
    case PortId::VuIn:
        vuIn = (float*)data;
        if (vuIn)
        {
            *vuIn = -96;
        }
        break;
    case PortId::LfoRate:
        lfoRate = (const float*)data;
        break;
    case PortId::LfoDepth:
        lfoDepth = (const float*)data;
        break;
    case PortId::LfoOut:
        lfoOut = (float*)data;
        if (lfoOut)
        {
            *lfoOut = 0;
        }
        break;
    case PortId::VuOutL:
        vuOutL = (float*)data;
        if (vuOutL)
        {
            *vuOutL = -96;
        }
        break;
    case PortId::VuOutR:
        vuOutR = (float*)data;
        if (vuOutR) {
            *vuOutR = -96;
        }
        break;
    case PortId::ToneStack:
        toneStack = (const float*)data;
        break;
    case PortId::Bass:
        bass = (const float*)data;
        break;
    case PortId::Mid:
        mid = (const float*)data;
        break;
    case PortId::Treble:
        treble = (const float*)data;
        break;
    case PortId::AudioInLeft:
        inLeft = (const float*)data;
        break;
    case PortId::AudioOutLeft:
        outL = (float*)data;
        break;
    case PortId::AudioOutRight:
        outR = (float*)data;
        break;
    default:
        LogError("Invalid port.");
        break;
    }
}
void SamplePlugin::Activate()
{
    lfoPhase = 0;
    // lfoRate is in Hz.

    // not actually implemented. For demonstration purposes only.
    // Should reset EQ filters here. 
    UpdateEq();

}
void SamplePlugin::Run(uint32_t n_samples)
{
    // Handle input controls. 
    // Test for changes if there are expensive computations.

    double lfoDx = 2*std::numbers::pi * *(this->lfoRate)/this->getRate();

    if (lastLevel != *level)
    {
        lastLevel = *level;
        amplitude =  (float)std::pow(10,(*this->level)*0.05); // convert from dB.
    }
    if (lastBass != *bass || lastTreble != *treble || lastMid != *mid || lastToneStack != *toneStack)
    {
        lastBass = *bass ;  lastTreble = *treble ;  lastMid = *mid ;  lastToneStack = *toneStack;
        // not actually implemented. For demonstration purposes only.
        UpdateEq();
    }

    // generate audio.
    float lfoDepth = *(this->lfoDepth);

    const float *in = this->inLeft;
    float *outL = this->outL;
    float *outR = this->outR;

    float maxInput = 0;
    float maxOutputL = 0;
    float maxOutputR = 0;

    for (size_t i = 0; i < n_samples; ++i)
    {

        this->lfoPhase += lfoDx;
        lfoPhase  = fmod(lfoPhase,2*std::numbers::pi);
    
        float lfoValueL = 
            lfoDepth*
                (1+(float)std::sin(lfoPhase))
                *0.5
            +(1-lfoDepth);
        float lfoValueR = 
            lfoDepth*
                (1+(float)std::cos(lfoPhase))
                *0.5
            +(1-lfoDepth);
              

        float inValue = in[i]*amplitude;
        float inAbs = std::abs(inValue);
        if (inAbs > maxInput) maxInput = inAbs;

        float valueL  = lfoValueL *inValue ;

        outL[i] = valueL;
        float valueR = lfoValueR * inValue;
        if (outR)
        {
            outR[i] = valueR;
        }

        float absValueL = std::abs(valueL);
        if (absValueL > maxOutputL)
        {
            maxOutputL = absValueL;
        }
        float absValueR = std::abs(valueR);
        if (absValueR > maxOutputR)
        {
            maxOutputR = absValueR;
        }
    }
    // output controls.
    *lfoOut = (float)std::sin(lfoPhase)*lfoDepth; //(something pretty to display)

    *vuIn =  std::log10(maxInput)*20;
    
    *vuOutL = std::log10(maxOutputL)*20;
    *vuOutR = std::log10(maxOutputR)*20;
} 

void SamplePlugin::Deactivate()
{
}

void SamplePlugin::UpdateEq()
{
    lastBass = *bass ;  lastTreble = *treble ;  lastMid = *mid ;  lastToneStack = *toneStack;

    // not implemented. for demonstration purposes only.
}