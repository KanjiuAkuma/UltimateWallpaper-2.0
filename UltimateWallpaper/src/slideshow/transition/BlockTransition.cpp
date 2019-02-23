#include "BlockTransition.h"

#include <glm/gtc/matrix_transform.hpp>

#include "util/Random.h"

BlockTransition::BlockTransition(const float duration, const float brightnessFilter, const int gridCellsX, const int gridCellsY, const float diffuseMin, const float diffuseMax, const float initialProgress)
	: Transition(1.f / duration, duration, "Block", initialProgress), m_gridCellsX(gridCellsX), m_gridCellsY(gridCellsY), m_diffuseMin(diffuseMin), m_diffuseMax(diffuseMax)
{
	using namespace Renderer;

	float vertices[]
	{
		//	pos     , tex
			0.f, 0.f, 0.f, 0.f, // left lower
			1.f, 0.f, 1.f, 0.f, // right lower
			1.f, 1.f, 1.f, 1.f, // right upper
			0.f, 1.f, 0.f, 1.f, // left upper
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

	m_shader = Shader::fromFiles("resources/shaders/slideShow/blockTransitionVertex.shader", "resources/shaders/slideShow/blockTransitionFragment.shader");

	float* diffuse = rndArray(m_gridCellsX * m_gridCellsY, m_diffuseMin, m_diffuseMax);
	m_diffuseBuffer = new TextureBuffer(diffuse, m_gridCellsX * m_gridCellsY, TextureBuffer::FLOAT);
	m_diffuseBuffer->bind();
	delete[] diffuse;

	m_shader->bind();
	m_shader->setUniform1f("u_brightnessFilter", brightnessFilter);
	m_shader->setUniform1i("u_diffuse", m_diffuseBuffer->getSlot());
	m_shader->setUniform1i("u_cellsX", m_gridCellsX);
	m_shader->setUniform1i("u_cellsY", m_gridCellsY);
}

BlockTransition::~BlockTransition() {
	delete m_mesh;
	delete m_shader;
	delete m_diffuseBuffer;
}

void BlockTransition::setTex0Slot(const unsigned int tex0Slot) {
	m_shader->bind();
	m_shader->setUniform1i("u_tex0", tex0Slot);
}

void BlockTransition::setTex1Slot(const unsigned int tex1Slot) {
	m_shader->bind();
	m_shader->setUniform1i("u_tex1", tex1Slot);
}

void BlockTransition::setDuration(const float duration) {
	m_duration = duration;
	m_speed = 1.f / duration;
}

void BlockTransition::setBrightnessFilter(const float brightnessFilter) {
	m_shader->bind();
	m_shader->setUniform1f("u_brightnessFilter", brightnessFilter);
}

void BlockTransition::setCellsX(const int cellsX) {
	m_gridCellsX = cellsX;
	m_shader->bind();
	m_shader->setUniform1i("u_cellsX", cellsX);
	float* diffuse = rndArray(m_gridCellsX * m_gridCellsY, m_diffuseMin, m_diffuseMax);
	m_diffuseBuffer->setData(diffuse, m_gridCellsX * m_gridCellsY);
	delete diffuse;
}

void BlockTransition::setCellsY(const int cellsY) {
	m_gridCellsY = cellsY;
	m_shader->bind();
	m_shader->setUniform1i("u_cellsY", cellsY);
	float* diffuse = rndArray(m_gridCellsX * m_gridCellsY, m_diffuseMin, m_diffuseMax);
	m_diffuseBuffer->setData(diffuse, m_gridCellsX * m_gridCellsY);
	delete diffuse;
}

void BlockTransition::setDiffuseMin(const float diffuseMin) {
	m_diffuseMin = diffuseMin;
	float* diffuse = rndArray(m_gridCellsX * m_gridCellsY, m_diffuseMin, m_diffuseMax);
	m_diffuseBuffer->setData(diffuse, m_gridCellsX * m_gridCellsY);
	delete diffuse;
}

void BlockTransition::setDiffuseMax(const float diffuseMax) {
	m_diffuseMax = diffuseMax;
	float* diffuse = rndArray(m_gridCellsX * m_gridCellsY, m_diffuseMin, m_diffuseMax);
	m_diffuseBuffer->setData(diffuse, m_gridCellsX * m_gridCellsY);
	delete diffuse;
}

void BlockTransition::render(const glm::mat4 mvp, const float bassAmplitude, const float highAmplitude) const {
	m_mesh->bind();
	m_shader->bind();
	m_shader->setUniform1f("u_transition", m_progress);
	m_shader->setUniform1f("u_highAmplitude", highAmplitude);
	m_shader->setUniformMat4("u_mvp", mvp * glm::scale(glm::mat4(1.f), glm::vec3(1.f + bassAmplitude, 1.f + bassAmplitude, 1.f)));

	GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_mesh->getVertexCount(), GL_UNSIGNED_INT, nullptr, m_gridCellsX * m_gridCellsY));
}
