#pragma once

#include "Spectrum.h"
#include <bass.h>

class AudioStreamReader final {
public:
	~AudioStreamReader();

	void updateSpectrum() const;
	Spectrum* getSpectrum() const;

	static AudioStreamReader* getInstance();

private:
	AudioStreamReader();

	DWORD m_bassFftSize = BASS_DATA_FFT8192;
	int m_fftSize = 8192;
	Spectrum* m_spectrum;
};
