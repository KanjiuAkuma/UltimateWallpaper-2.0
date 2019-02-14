#pragma once
#include "Spectrum.h"
#include "AudioStreamReader.h"

class SpectrumPreProcessor {
public:
	virtual ~SpectrumPreProcessor();
	virtual void updateSpectrum();

	Spectrum* getSpectrum() const;

protected:
	AudioStreamReader* m_inputStreamReader = AudioStreamReader::getInstance();
	Spectrum* m_inputSpectrum = AudioStreamReader::getInstance()->getSpectrum();
	Spectrum* m_spectrum = new Spectrum(m_inputSpectrum->size, m_inputSpectrum->sampleRate);
};
