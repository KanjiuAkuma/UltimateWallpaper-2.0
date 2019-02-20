#pragma once

#include <boost/property_tree/ptree.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <renderer/Renderer.h>

#include "util/audioAnalyzer/Spectrum.h"
#include "editor/Configurable.h"

class Equalizer final : public Configurable {

	int m_bars = 16;
	float m_colorTime = 0.f, m_rotationTime = 0.f, m_colorFrequency = 0.f, m_rotationFrequency = 0.f;
	float m_colorOffset = 0.f;
	float m_baseAmplifier = 5.f, m_peakAmplifier = .3f;
	Spectrum* m_audioSpectrum;

	JApp::Renderer::Shader* m_shader = nullptr;
	JApp::Renderer::Mesh* m_mesh = nullptr;
	JApp::Renderer::TextureBuffer* m_spectrumBuffer = nullptr;

	glm::vec3 m_position = glm::vec3(0.f, 0.f, 0.f);
	bool m_flip = false;
	glm::vec3 m_size = glm::vec3(1.f, 1.f, 1.f);
	float m_angle = 0.f;
	glm::mat4 m_model = glm::mat4(1.f);

public:
	explicit Equalizer(Spectrum* spectrum);
	~Equalizer();

	void update(float dt);
	void render(glm::mat4 mvp) const;

	void loadSettings(boost::property_tree::ptree& cfg) override;

	#if SHOW_EDITOR
	void setBarCount(int barCount);
	void setBarWidth(float barWidth) const;
	void setBaseAmplitude(float baseAmplitude) const;
	void setColorOffset(float offset);
	void setAlpha(float alpha) const;
	void disableColorFlow();
	void setColorFlowSpeed(float speed);
	void setPositionX(float x);
	void setPositionY(float y);
	void setAngle(float degrees);
	void setFlip(bool flip);
	void setWidth(float width);
	void setHeight(float height);
	void setInnerRadius(float innerRadius) const;
	void setOuterRadius(float outerRadius) const;
	void setInnerRounding(bool enable) const;
	void setOuterRounding(bool enable) const;
	void disableRotation();
	void setRotationSpeed(float speed);
	void setBaseAmplifier(float baseAmplifier);
	void setPeakAmplifier(float peakAmplifier);
	#endif
};
