#include "Particle.h"


Particle::Particle(
	const float radius, const float glowSize, const float glowAmount,
	const glm::vec3& color,
	const glm::vec2& position, const glm::vec2& velocity,
	const float angle, const float angularVelocity)
	:
	radius(radius), glowSize(glowSize), glowAmount(glowAmount),
	color(color),
	position(position), velocity(velocity),
	angle(angle), angularVelocity(angularVelocity) {
	if (velocity.x == 0 && velocity.y == 0) {
		velocityMagnitude = 0;
	}
	else {
		velocityMagnitude = length(velocity);
	}
}