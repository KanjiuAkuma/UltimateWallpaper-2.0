#pragma once

#define LOW_END 10.f
#define SUB_BASS 60.f
#define BASS 250.f
#define LOW_MID_RANGE 500.f
#define MID_RANGE 2000.f
#define UPPER_MID_RANGE 4000.f
#define PRESENCE 6000.f
#define BRILLIANCE 20000.f
#define HIGH_END 24000.f

struct Spectrum {

	const int size, sampleRate;
	const float nyquist;

	float* data;

	explicit Spectrum(const int size, const int sampleRate) : size(size), sampleRate(sampleRate), nyquist(float(sampleRate) / 2.f), data(new float[size]) {}
	virtual ~Spectrum() {
		delete[] data;
	}

	float convertIndex(int index) const;
	int convertFrequency(float frequency) const;

	float sumRange(float f0, float f1, bool average = false) const;
	float sumRange(float f0, float f1, float amplifier) const;
};
