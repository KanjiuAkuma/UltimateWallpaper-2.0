#include "AlphaTransition.h"

#include <glm/gtc/matrix_transform.hpp>

AlphaTransition::AlphaTransition(const float duration, const float brightnessFilter, const float initialProgress)
	: Transition(1.f / duration, duration, "Alpha", initialProgress)
{
	using namespace Renderer;

	float vertices[]
	{
		//	 pos      , tex
			-1.f, -1.f, 0.f, 0.f, // left lower
			 1.f, -1.f, 1.f, 0.f, // right lower
			 1.f,  1.f, 1.f, 1.f, // right upper
			-1.f,  1.f, 0.f, 1.f, // left upper
	};

	auto* va = new VertexArray();
	auto* vb = new VertexBuffer(vertices, 4 * 4 * sizeof(float));
	auto* vbl = new VertexBufferLayout();
	vbl->push<float>(2);
	vbl->push<float>(2);

	va->addBuffer(vb, vbl);

	unsigned int indices[]
	{
		0, 1, 2,
		2, 3, 0,
	};

	auto* ib = new IndexBuffer(indices, 6);

	m_mesh = new Mesh(va, vb, vbl, ib);

	m_shader = Shader::fromFiles("resources/shaders/slideShow/alphaTransitionVertex.shader", "resources/shaders/slideShow/alphaTransitionFragment.shader");
	m_shader->bind();
	m_shader->setUniform1f("u_brightnessFilter", brightnessFilter);
}
AlphaTransition::~AlphaTransition() {
	delete m_mesh;
	delete m_shader;
}

void AlphaTransition::setTex0Slot(const unsigned int tex0Slot) {
	m_shader->bind();
	m_shader->setUniform1i("u_tex0", tex0Slot);
}

void AlphaTransition::setTex1Slot(const unsigned int tex1Slot) {
	m_shader->bind();
	m_shader->setUniform1i("u_tex1", tex1Slot);
}

void AlphaTransition::setDuration(const float duration) {
	m_duration = duration;
	m_speed = 1.f / duration;
}

void AlphaTransition::setBrightnessFilter(const float brightnessFilter) {
	m_shader->bind();
	m_shader->setUniform1f("u_brightnessFilter", brightnessFilter);
}

void AlphaTransition::render(const glm::mat4 mvp, const float bassAmplitude, const float highAmplitude) const {
	// APP_INFO("L={:.5f}, H={:.5f}", bassAmplitude, highAmplitude);
	m_mesh->bind();
	m_shader->bind();
	m_shader->setUniform1f("u_transition", m_progress);
	// m_shader->setUniform1f("u_highAmplitude", 0.f);
	m_shader->setUniform1f("u_highAmplitude", highAmplitude);
	// m_shader->setUniformMat4("u_mvp", mvp);
	m_shader->setUniformMat4("u_mvp", mvp * glm::scale(glm::mat4(1.f), glm::vec3(1.f + bassAmplitude, 1.f + bassAmplitude, 1.f)));

	GL_CALL(glDrawElements(GL_TRIANGLES, m_mesh->getVertexCount(), GL_UNSIGNED_INT, nullptr));
}
