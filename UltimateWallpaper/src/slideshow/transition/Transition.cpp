#include "Transition.h"

Transition::Transition(const float speed, const float duration, const std::string& type)
	: m_speed(speed), m_duration(duration), m_progress(0), m_type(type) {}

float Transition::getDuration() {
	return m_duration;
}

std::string Transition::getType() const {
	return m_type;
}

void Transition::update(const float dt) {
	m_progress += dt * m_speed;
}

void Transition::goToStart() {
	m_progress = 0.f;
}

void Transition::goToEnd() {
	m_progress = m_duration;
}
