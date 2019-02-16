#pragma once

#include <application/Application.h>

#include <boost/property_tree/ptree.hpp>

#include "slideshow/SlideShow.h"
#include "particleEffect/ParticleEffect.h"
#include "util/audioAnalyzer/SmoothSpectrumPreProcessor.h"

using namespace JApp;

class UltimateWallpaper : public Application {
public:
	explicit UltimateWallpaper(boost::property_tree::ptree& configuration);
	virtual ~UltimateWallpaper();

	void render() override;
	void update(float dt) override;

	void renderFpsCounter(bool newFrame) const;

protected:
	bool m_fpsCounterEnable;
	float m_fpsCounterPosX, m_fpsCounterPosY;
	float m_sceneWidth = 1.f, m_sceneHeight = 1.f;

	AudioStreamReader* m_audioStreamReader = AudioStreamReader::getInstance();
	SmoothSpectrumPreProcessor* m_audioPreProcessor;
	SlideShow* m_slideShow = nullptr;
	ParticleEffect* m_particleEffect = nullptr;
};
