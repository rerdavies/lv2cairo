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

#pragma once

#include "Lv2Plugin.hpp"

using namespace lv2;

class SamplePlugin : public Lv2Plugin
{
public:
	using super = Lv2Plugin;

	static Lv2Plugin *Create(double rate,
							 const char *bundle_path,
							 const LV2_Feature *const *features)
	{
		return new SamplePlugin(rate, bundle_path, features);
	}
	SamplePlugin(double rate,
				 const char *bundle_path,
				 const LV2_Feature *const *features);

protected:
	virtual void ConnectPort(uint32_t port, void *data) override;
	virtual void Activate() override;
	virtual void Run(uint32_t n_samples) override;
	virtual void Deactivate() override;

private:
	const float *level = nullptr;
	float *vuIn = nullptr;
	float *vuOutL = nullptr;
	float *vuOutR = nullptr;
	const float *lfoRate = nullptr;
	const float *lfoDepth = nullptr;
	float *lfoOut = nullptr;
	const float *toneStack = nullptr;

	const float *bass = nullptr;
	const float *mid = nullptr;
	const float *treble = nullptr;
	const float *inLeft = nullptr;
	float *outL = nullptr;
	float *outR = nullptr;

	float amplitude = 1.0;
	float lfoPhase = 0;
	static constexpr double UNINITIALIZED = 1E-180;
	float lastLevel = UNINITIALIZED;
	float lastToneStack = UNINITIALIZED;
	float lastTreble = UNINITIALIZED;
	float lastMid = UNINITIALIZED;
	float lastBass = UNINITIALIZED;

private:
	enum class PortId
	{
		Level = 0,
		VuIn,
		LfoRate,
		LfoDepth,
		LfoOut,
		VuOutL,
		VuOutR,

		Bass,
		Mid,
		Treble,
		ToneStack,

		AudioInLeft,
		AudioOutLeft,
		AudioOutRight,
	};

	void UpdateEq();
};