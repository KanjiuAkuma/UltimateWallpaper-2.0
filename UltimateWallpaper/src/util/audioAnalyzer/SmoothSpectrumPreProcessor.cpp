#include "SmoothSpectrumPreProcessor.h"

SmoothSpectrumPreProcessor::SmoothSpectrumPreProcessor(const float smoothingFactor, const float threshold):
	m_smoothingFactor(smoothingFactor), m_threshold(threshold), m_recentValues(new float[m_inputSpectrum->size - 1]) {
	for (int i = 0; i < m_inputSpectrum->size - 1; i++) {
		m_recentValues[i] = 0;
	}
}

SmoothSpectrumPreProcessor::~SmoothSpectrumPreProcessor() {
	delete[] m_recentValues;
}

void SmoothSpectrumPreProcessor::updateSpectrum() {
	for (int i = 0; i < m_spectrum->size - 1; i++) {
		const float fqIn = m_inputSpectrum->data[i + 1];
		const float fqS = fqIn - m_recentValues[i];
		if (fqS < m_threshold * m_recentValues[i]) m_spectrum->data[i] = 0;
		else m_spectrum->data[i] = fqS - m_threshold * m_recentValues[i];
		m_recentValues[i] = (m_recentValues[i] * m_smoothingFactor + fqIn) / (m_smoothingFactor + 1);
	}
}
