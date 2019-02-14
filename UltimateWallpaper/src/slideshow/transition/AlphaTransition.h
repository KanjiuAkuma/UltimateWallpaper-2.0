#pragma once
#include "Transition.h"
#include "renderer/Renderer.h"

using namespace JApp;

class AlphaTransition : public Transition {
	Renderer::Mesh* m_mesh;
	Renderer::Shader* m_shader;

public:
	explicit AlphaTransition(float duration);
	~AlphaTransition() override;

	void setTex0Slot(unsigned int tex0Slot) override;
	void setTex1Slot(unsigned int tex1Slot) override;

	// settings
	void setDuration(float duration) override;

	void render(glm::mat4 mvp, float bassAmplitude, float highAmplitude) const override;
};
