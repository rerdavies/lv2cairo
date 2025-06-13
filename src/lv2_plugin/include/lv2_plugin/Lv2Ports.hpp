/*
 *   Copyright (c) 2022 Robin E. R. Davies
 *   All rights reserved.

 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#pragma once

#include <limits>
#include <cstdint>
#include <cmath>
#include "lv2/atom/atom.h"



namespace lv2c::lv2_plugin 
{
	#ifndef AF2DB_H_
	#define AF2DB_H_

	inline float AF2Db(float value) {
		//  af = pow(10,db/20) = exp(log(10)*db/20)
		// log(af) = log(10)*db/20
		// db = log(af)*20/log(10)
		// K = 20/log(10)
		if (value < 2.5118864e-10f) return -192.0f;
		constexpr float K = 8.68588963807;
		return std::log(value)*K;
	}

	inline float Db2AF(float db, float minValue)
	{
		if (db == minValue) {
			return 0;
		}
		// pow(10,db/20)
		// exp(log(10)/20*db)
		// exp(K*db) where 
		// K = log(10)/20.   But log is not constexpr until c++26.
		constexpr float K = 0.11512925465f;

		return std::exp(K*db);

	}

	inline float Db2Af(double db, float minValue)
	{
		if (db == minValue) {
			return 0;
		}
		// pow(10,db/20)
		// exp(log(10)/20*db)
		// exp(K*db) where 
		// K = log(10)/20.   But log is not constexpr until c++26.
		constexpr double K = 0.11512925465f;

		return std::exp(K*db);

	}
	#endif

	class InputPort {
	private:
		const float* pData = nullptr;
		float lastValue = -std::numeric_limits<float>::max();

	public:
		void SetData(void* data) {
			pData = (float*)data;
		}
		bool HasChanged()
		{
			if (pData == nullptr) return false;
			return (*pData != lastValue);
		}

		float GetValue()
		{
			return lastValue = *pData;
		}
	};

	class ToggledInputPort {
	private:
		const float *pData = nullptr;
		bool lastValue = false;
	public:
		void SetData(void *data) {
			this->pData = (const float*)data;
			this->lastValue = false;
		}

		bool HasChanged()
		{
			bool newValue = (*pData != 0);
			bool result = newValue != lastValue;
			lastValue = newValue;
			return result;
		}

		bool GetValue() {
			bool newValue = (*pData != 0);
			lastValue = newValue;
			return newValue;
		}
	};

	class TriggerInputPort {
	private:
		const float *pData = nullptr;
		bool lastValue = false;
        float defaultValue;
	public:
        TriggerInputPort(float defaultValue)
        : defaultValue(defaultValue) {
        }
		void SetData(void *data) {
			this->pData = (const float*)data;
		}

		bool IsTriggered()
		{
			bool newValue = (*pData != 0);
			bool changed = newValue != lastValue;
			lastValue = newValue;
			return changed && newValue != defaultValue; // rising edge only.
		}
	};

	class RangedInputPort {
	private:
		float minValue, maxValue;
		const float* pData = nullptr;
		float lastValue = -std::numeric_limits<float>::max();

	private:
		float ClampedValue()
		{
			float v = *pData;
			if (v < minValue) v = minValue;
			if (v > maxValue) v = maxValue;
			return v;
		}
	public:
		RangedInputPort(float minValue, float maxValue)
		{
			this->minValue = minValue;
			this->maxValue = maxValue;
		}
		float GetMaxValue() { return this->maxValue; }
		float GetMinValue() { return this->minValue; }

		void SetData(void* data) {
			pData = (float*)data;
		}
		bool HasChanged()
		{
			// fast path for well-behaved hosts that trim inputs.
			if (*pData == lastValue) return false; 

			// make sure the host hasn't given us an out of range value.
			return ClampedValue() != lastValue;
		}

		float GetValue()
		{
			return lastValue = ClampedValue();
		}

	};
	class EnumeratedInputPort {
	private:
		int32_t nValues;
		const float* pData = nullptr;
		float lastValue = -std::numeric_limits<float>::max();

	private:
		int32_t ClampedValue()
		{
			float vFloat = *pData;
			if (vFloat < 0) vFloat = 0;
			if (vFloat > nValues-1) vFloat = nValues-1;
			int32_t v = (int32_t)std::floor(vFloat+0.5f);
			return v;
		}
	public:
		EnumeratedInputPort(int32_t nValues)
		:nValues(nValues)
		{
		}

		void SetData(void* data) {
			pData = (float*)data;
		}
		bool HasChanged()
		{
			// fast path for well-behaved hosts that trim inputs.
			return (*pData != lastValue);

		}

		int32_t GetValue()
		{
			lastValue = *pData;
			return ClampedValue();
		}

	};

	class RangedDbInputPort {
	private:
		float minValue, maxValue;
		const float* pData = nullptr;
		float lastValue = -std::numeric_limits<float>::max();
		float lastAfValue = 0;

	private:
		float ClampedValue()
		{
			float v = *pData;
			if (v < minValue) v = minValue;
			if (v > maxValue) v = maxValue;
			return v;
		}
	public:
		RangedDbInputPort(float minValue, float maxValue)
		{
			this->minValue = minValue;
			this->maxValue = maxValue;
		}
		float GetMinDb() const { return minValue; }
		float GetMaxDb() const{ return maxValue; }

		void SetData(void* data) {
			pData = (float*)data;
		}
		bool HasChanged()
		{
			if (*pData == lastValue) return false; // fast path for well-behaved hosts.

			// make sure the host hasn't given us an out of range value.
			return ClampedValue() != lastValue;
		}
		float GetDb()
		{
			if (HasChanged())
			{
				lastValue = ClampedValue();
				lastAfValue = Db2Af(lastValue,minValue);
			}
			return lastValue;
		}
		float GetDbNoLimit()
		{
			if (HasChanged())
			{
				lastValue = ClampedValue();
				lastAfValue = Db2Af(lastValue,-192);
			}
			return lastValue;
		}

		float GetAf()
		{
			if (HasChanged())
			{
				lastValue = ClampedValue();
				lastAfValue = Db2Af(lastValue,minValue);
			}
			return lastAfValue;
		}
		float GetAfNoLimit()
		{
			if (HasChanged())
			{
				lastValue = ClampedValue();
				lastAfValue = Db2Af(lastValue,-192);
			}
			return lastAfValue;
		}

	};

	class BooleanInputPort  {
	public:
		bool GetValue() const {
			return *pData > 0;
		}
		void SetData(void*pData)
		{
			this->pData = (float*)pData;
		}
	private:
		float *pData = nullptr;
	};

	class SteppedInputPort  {
	private:
		float *pData = nullptr;
		float lastValue;
		int minValue,maxValue;
	public:
		SteppedInputPort(int minValue, int maxValue)
		{
			this->minValue = minValue;
			this->maxValue = maxValue;
			this->lastValue = std::numeric_limits<float>::max();

		}
		bool HasChanged()
		{
			if (pData == nullptr) return false;
			return (*pData != lastValue);
		}

		int GetValue()
		{
			lastValue = (*pData);
			int result = (int)(lastValue+0.5);
			if (result < minValue) return minValue;
			if (result > maxValue) return  maxValue;
			return result;
		}
		void SetData(void*pData)
		{
			this->pData = (float*)pData;
		}

	};

	class AudioInputPort {
	public:
		void SetData(void*pData)
		{
			this->pData = (const float*)pData;
		}
		const float*Get() const { return pData; }
	private:
		const float *pData = nullptr;
	};

	class AudioOutputPort {
	public:
		void SetData(void*pData)
		{
			this->pData = (float*)pData;
		}
		float*Get() const { return pData; }
	private:
		float *pData = nullptr;
	};

	class AtomInputPort {
	public:
		void SetData(void*pData)
		{
			this->pData = (void*)pData;
		}
		LV2_Atom_Sequence*Get() const { return (LV2_Atom_Sequence*)pData; }
	private:
	void *pData = nullptr;
	};

	class AtomOutputPort {
	public:
		void SetData(void*pData)
		{
			this->pData = ( void*)pData;
		}
		LV2_Atom_Sequence*Get() const { return (LV2_Atom_Sequence*)pData; }
	private:
	void *pData = nullptr;
	};


	class OutputPort
	{
	private:
		float *pOut = 0;
		float defaultValue;

	public:
		explicit OutputPort(float defaultValue = 0)
		{
			this->defaultValue = defaultValue;
		}
		void SetData(void *data)
		{
			if (pOut != nullptr)
			{
				defaultValue = *pOut;
			}
			pOut = (float *)data;
			if (pOut != nullptr)
			{
				*pOut = defaultValue;
			}
		}

		void SetValue(float value)
		{
			if (pOut != 0)
			{
				*pOut = value;
			}
			else
			{
				defaultValue = value;
			}
		}
	};

	class RateLimitedOutputPort
	{
	private:
		float *pOut = 0;
		float updateRateHz;
		size_t updateRate;
		size_t sampleCount = 0;
		float lastValue = 0;

	public:
		RateLimitedOutputPort(float defaultValue = 0,float updateRateHz = 15.0f)
			: updateRateHz(updateRateHz)
		{
			lastValue = defaultValue;
		}
		void SetSampleRate(double sampleRate)
		{
			updateRate = (size_t)(sampleRate / updateRateHz);
		}
		void Reset(double value)
		{
			sampleCount = 0;
			lastValue = value;
			if (pOut)
			{
				*pOut = value;
			}
		}
		void SetData(void *data)
		{
			pOut = (float *)data;
			if (pOut != nullptr)
			{
				*pOut = lastValue;
			}
		}

		// Sets value immediately.
		void SetValue(float value)
		{
			lastValue = value;
			*pOut = lastValue;
			sampleCount = 0;
		}

		// Sets value, throttles update. 
		void SetValue(
			float value, 
			size_t n_values // number of samples in the current frame.
			)
		{
			lastValue = value;
			sampleCount += n_values;
			if (sampleCount >= updateRate)
			{
				sampleCount = 0;
				if (pOut)
				{
					*pOut = lastValue;
				}
			}
		}
	};

	class VuOutputPort
	{
	private:
		float *pOut = 0;
		float minDb, maxDb;
		size_t updateRate;
		size_t sampleCount = 0;
		float maxValue = 0;

	public:
		VuOutputPort(float minDb, float maxDb)
		{
			this->minDb = minDb;
			this->maxDb = maxDb;
			this->maxValue = minDb;
		}
		void SetSampleRate(double sampleRate)
		{
			updateRate = (size_t)(sampleRate / 30);
			Reset();
		}
		void Reset()
		{
			// update 30 times a second.
			sampleCount = 0;
			maxValue = 0;
			if (pOut)
			{
				*pOut = minDb;
			}
		}
		void SetData(void *data)
		{
			pOut = (float *)data;
			if (pOut != nullptr)
			{
				*pOut = minDb;
			}
		}

		void AddValue(float value)
		{
			auto t = std::abs(value);
			if (t > maxValue)
			{
				maxValue = t;
			}
			if (++sampleCount >= updateRate)
			{
				sampleCount -= updateRate;
				if (pOut)
				{
					float value = AF2Db(maxValue);
					if (value < minDb)
					{
						value = minDb;
					}
					if (value > maxDb)
						value = maxDb;
					*pOut = value;
				}
				maxValue = 0;
			}
		}
		void AddValues(size_t count, float *values)
		{

			for (size_t i = 0; i < count; ++i)
			{
				auto t = std::abs(values[i]);
				if (t > maxValue)
				{
					maxValue = t;
				}
			}
			sampleCount += count;
			if (sampleCount >= updateRate)
			{
				sampleCount -= updateRate;
				if (pOut)
				{
					float value = AF2Db(maxValue);
					if (value < minDb)
						value = minDb;
					if (value > maxDb)
						value = maxDb;
					*pOut = value;
				}
				maxValue = 0;
			}
		}
	};	
}

