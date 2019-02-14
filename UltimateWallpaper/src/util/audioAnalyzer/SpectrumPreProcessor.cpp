#include "SpectrumPreProcessor.h"

SpectrumPreProcessor::~SpectrumPreProcessor() {
	delete m_spectrum;
}

void SpectrumPreProcessor::updateSpectrum() {
	memcpy(m_spectrum->data, m_inputSpectrum->data, m_inputSpectrum->size * sizeof(float));
}

Spectrum* SpectrumPreProcessor::getSpectrum() const {
	return m_spectrum;
}
