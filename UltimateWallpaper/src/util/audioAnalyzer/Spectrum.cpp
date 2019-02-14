#include "Spectrum.h"

#include <cmath>

float Spectrum::convertIndex(const int index) const {
	return float(index) / float(size - 1) * nyquist;
}

int Spectrum::convertFrequency(const float frequency) const {
	const int idx = int((size - 1) * frequency / nyquist);
	if (idx < 1) return 1;
	if (size < idx) return size - 1;
	return idx;
}

float Spectrum::sumRange(const float f0, const float f1, const bool average) const {
	float sum = 0;
	const int i0 = convertFrequency(f0);
	const int i1 = convertFrequency(f1);


	for (int i = i0; i < i1; i++) {
		sum += data[i];
	}

	if (average && i0 != i1) {
		sum /= i1 - i0;
	}

	return sum;
}

float Spectrum::sumRange(const float f0, const float f1, const float amplifier) const {
	float sum = 0;
	const int i0 = convertFrequency(f0);
	const int i1 = convertFrequency(f1);

	
	for (int i = i0; i < i1; i++) {
		sum += pow(data[i], amplifier);
	}
	sum = pow(sum, 1.f / amplifier);

	return sum;
}
