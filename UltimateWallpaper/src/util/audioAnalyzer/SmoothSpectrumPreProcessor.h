#pragma once
#include "SpectrumPreProcessor.h"

class SmoothSpectrumPreProcessor final : public SpectrumPreProcessor {
public:
	explicit SmoothSpectrumPreProcessor(float smoothingFactor, float threshold);
	~SmoothSpectrumPreProcessor() override;
	
	void setSmoothingFactor(float smoothingFactor);
	void setThreshold(float threshold);

	void updateSpectrum() override;

private:
	float m_smoothingFactor, m_threshold;
	float *m_recentValues;
};
