#pragma once
#include <glm/glm.hpp>
#include <renderer/Renderer.h>

struct ParticleModel
{
	glm::vec2* corners = new glm::vec2[3];
	JApp::Renderer::Mesh *coreMesh;

	explicit ParticleModel();
	~ParticleModel();
};