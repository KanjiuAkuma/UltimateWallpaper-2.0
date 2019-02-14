#pragma once
#include "SpectrumPreProcessor.h"

class SmoothSpectrumPreProcessor final : public SpectrumPreProcessor {
public:
	explicit SmoothSpectrumPreProcessor(float smoothingFactor, float threshold);
	~SmoothSpectrumPreProcessor() override;
	void updateSpectrum() override;

private:
	float m_smoothingFactor, m_threshold;
	float *m_recentValues;
};
