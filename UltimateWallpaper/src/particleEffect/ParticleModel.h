#pragma once
#include <glm/glm.hpp>
#include <renderer/Renderer.h>

using namespace JApp;

struct ParticleModel
{
	glm::vec2* corners = new glm::vec2[3];
	Renderer::Mesh *coreMesh;

	explicit ParticleModel();
	~ParticleModel();
};