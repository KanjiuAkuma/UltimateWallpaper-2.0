#pragma once
#include "ParticleModel.h"

struct Particle {
	float radius, glowSize, glowAmount;
	glm::vec3 color;
	glm::vec2 position, velocity;
	float velocityMagnitude;
	float angle, angularVelocity;

	Particle(float radius, float glowSize, float glowAmount, const glm::vec3& color, const glm::vec2& position, const glm::vec2& velocity, float angle, float angularVelocity);
};
