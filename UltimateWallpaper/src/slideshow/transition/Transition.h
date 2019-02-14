#pragma once

#include <glm/glm.hpp>
#include <string>

/*
 * Settings:
 *		Duration
 */

class Transition {
public:
	Transition(float speed, float duration, const std::string& type);
	virtual ~Transition() = default;

	virtual float getDuration();
	virtual void setTex0Slot(unsigned int tex0Slot) = 0;
	virtual void setTex1Slot(unsigned int tex1Slot) = 0;

	// settings
	virtual void setDuration(float duration) = 0;
	std::string getType() const;

	virtual void update(float dt);
	virtual void render(glm::mat4 mvp, float bassAmplitude, float highAmplitude) const = 0;

	void goToStart();
	void goToEnd();
protected:
	float m_speed, m_duration, m_progress;
	std::string m_type;
};