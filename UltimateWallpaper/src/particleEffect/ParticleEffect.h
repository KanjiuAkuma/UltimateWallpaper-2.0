#pragma once

#include <boost/property_tree/ptree.hpp>

#include <renderer/Renderer.h>

#include "Particle.h"

#include "util/audioAnalyzer/Spectrum.h"
#include "editor/Configurable.h"

class ParticleEffect final : public Configurable {
	
	boost::property_tree::ptree m_cfg;

	int m_particleCount = 0;
	Particle** m_particles = nullptr;
	
	bool m_lineEnable = false, m_mouseGravityEnable = false;
	float m_mouseGravityStrength = 0;

	ParticleModel* m_particleModel = new ParticleModel();
	JApp::Renderer::TextureBuffer *m_particlesPositionsBuffer = nullptr,
							*m_particlesVariablesBuffer = nullptr, 
							*m_particlesColorsBuffer = nullptr;
	JApp::Renderer::Shader *m_coreShader;
	JApp::Renderer::Shader* m_lineShader;
	JApp::Renderer::VertexArray* m_lineVertexArray = new JApp::Renderer::VertexArray();

	bool m_audioResponseEnable = false;
	Spectrum* m_audioSpectrum;
	float m_bassBaseAmplifier = 0.f, m_bassPeakAmplifier = 0.f;
	float m_highBaseAmplifier = 0.f, m_highPeakAmplifier = 0.f;

public:
	ParticleEffect(Spectrum* spectrum);
	~ParticleEffect();

	void update(float dt, const glm::vec2& mousePos, float windowWidth, float windowHeight) const;
	void render(glm::mat4 mvp, const glm::vec2& mousePos) const;

	// settings
	void loadSettings(boost::property_tree::ptree& configuration) override;
	void setParticleCount(int particleCount);

	#if SHOW_EDITOR
	void setFixedSize(float size);
	void setSize(float sizeMin, float sizeMax);
	void reInitSizes() const;

	void setVelocity(float velMin, float velMax);
	void setVelocityX(float velMin, float velMax);
	void setVelocityY(float velMin, float velMax);
	void reInitVelocities() const;
	void setAngleVelocity(float velMin, float velMax);
	void reInitAngleVelocity() const;
	
	void setGlowEnable(bool enable) const;
	void setGlowSize(float glowSizeMin, float glowSizeMax);
	void reInitGlowSize() const;

	void setGlowAmount(float glowAmountMin, float glowAmountMax);
	void reInitGlowAmount() const;
	
	void setLineEnable(bool lineEnable);
	void setLineWidth(float lineWidth) const;
	void setLineMaxDistance(float lineMaxDistance) const;
	void setMouseGravityEnable(bool mouseGravityEnable);
	void setMouseGravityStrength(float mouseGravityStrength);

	void setAudioResponseEnable(bool audioResponseEnable);
	void setBassPeakAmplifier(float peakAmplifier);
	void setHighPeakAmplifier(float peakAmplifier);
	void setBassBaseAmplifier(float baseAmplifier);
	void setHighBaseAmplifier(float baseAmplifier);
	void setColorFilter(float colorFilter) const;
	#endif

private:

	static Particle* createParticle(
		float sizeMin, float sizeMax,
		float glowSizeMin, float glowSizeMax, float glowAmountMin, float glowAmountMax,
		float velocityMinX, float velocityMaxX,
		float velocityMinY, float velocityMaxY,
		float wMin, float wMax);
};
