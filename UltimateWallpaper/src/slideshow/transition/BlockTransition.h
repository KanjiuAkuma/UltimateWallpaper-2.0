#pragma once
#include "Transition.h"
#include "renderer/Renderer.h"

using namespace JApp;

class BlockTransition final : public Transition {
	Renderer::Mesh* m_mesh;
	Renderer::Shader* m_shader;
	Renderer::TextureBuffer* m_diffuseBuffer;
	int m_gridCellsX, m_gridCellsY;
	float m_diffuseMin, m_diffuseMax;

public:
	explicit BlockTransition(float duration, float brightnessFilter, int gridCellsX, int gridCellsY, float diffuseMin, float diffuseMax, float initialProgress = 0.f);
	~BlockTransition() override;

	void setTex0Slot(unsigned int tex0Slot) override;
	void setTex1Slot(unsigned int tex1Slot) override;

	// settings
	void setDuration(float duration) override;
	void setBrightnessFilter(float brightnessFilter) override;
	void setCellsX(int cellsX);
	void setCellsY(int cellsY);
	void setDiffuseMin(float diffuseMin);
	void setDiffuseMax(float diffuseMax);

	void render(glm::mat4 mvp, float bassAmplitude, float highAmplitude) const override;
};

