#include "ParticleEffect.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "util/Random.h"

ParticleEffect::ParticleEffect() {	
	using namespace Renderer;

	m_coreShader = Shader::fromFiles(
		"resources/shaders/particle/particleVertex.shader",
		"resources/shaders/particle/particleFragment.shader"
	);

	m_lineShader = Shader::fromFiles(
		"resources/shaders/particle/lineVertex.shader",
		"resources/shaders/particle/lineGeometry.shader",
		"resources/shaders/particle/lineFragment.shader"
	);
}

ParticleEffect::~ParticleEffect() {
	for (int i = 0; i < m_particleCount; i++) {
		delete m_particles[i];
	}
	delete[] m_particles;

	// rendering
	delete m_particleModel;
	delete m_particlesPositionsBuffer;
	delete m_particlesColorsBuffer;
	delete m_particlesVariablesBuffer;
	delete m_coreShader;
	delete m_lineShader;
	delete m_lineVertexArray;
}

void ParticleEffect::update(const float dt, const glm::vec2& mousePos, const float windowWidth, const float windowHeight) const {
	for (int i = 0; i < m_particleCount; i++) {
		Particle* p = m_particles[i];
		p->angle += p->angularVelocity * dt;
		p->angle = glm::mod(p->angle, 360.f);

		if (m_mouseGravityEnable && p->velocityMagnitude != 0) {
			// apply gravity

			const glm::vec2 r = p->position - mousePos;
			const float magnitude = glm::length(r);

			const glm::vec2 a = -m_mouseGravityStrength / magnitude * r;

			p->velocity += a * .5f * glm::pow(dt, 2.f);
			p->velocity = normalize(p->velocity) * p->velocityMagnitude;
		}
		
		p->position += p->velocity * dt;

		float rds = glm::radians(p->angle);

		glm::vec2 c0 = rotate(m_particleModel->corners[0], rds);
		glm::vec2 c1 = rotate(m_particleModel->corners[1], rds);
		glm::vec2 c2 = rotate(m_particleModel->corners[2], rds);

		const glm::vec2 min = glm::min(glm::min(c0, c1), c2) * p->radius + p->position;
		const glm::vec2 max = glm::max(glm::max(c0, c1), c2) * p->radius + p->position;

		if (min.x < -windowWidth) {
			p->position.x += -windowWidth - min.x;
			p->velocity.x *= -1;
		}
		else if (windowWidth < max.x) {
			p->position.x -= max.x - windowWidth;
			p->velocity.x *= -1;
		}

		if (min.y < -windowHeight) {
			p->position.y += -windowHeight - min.y;
			p->velocity.y *= -1;
		}
		else if (windowHeight < max.y) {
			p->position.y -= max.y - windowHeight;
			p->velocity.y *= -1;
		}
	}
}

void ParticleEffect::render(const glm::mat4 mvp, const glm::vec2& mousePos) const {
	float* ptr = m_particlesPositionsBuffer->getDataPointer();

	for (int i = 0; i < m_particleCount; i++) {
		Particle* p = m_particles[i];
		ptr[0] = p->position.x;
		ptr[1] = p->position.y;
		ptr[2] = p->angle;
		ptr += 3;
	}

	// append mouse as particle
	ptr[0] = mousePos.x;
	ptr[1] = mousePos.y;

	m_particlesPositionsBuffer->freeDataPointer();

	// lines
	if (m_lineEnable) {
		const int lineCount = (m_particleCount + 1) * m_particleCount / 2;
		m_lineVertexArray->bind();
		m_lineShader->bind();
		m_lineShader->setUniformMat4("u_mvp", mvp);
		GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, lineCount));
	}

	/* draw core */
	m_coreShader->bind();

	ParticleModel* m = m_particleModel;
	m->coreMesh->bind();
	
	m_coreShader->setUniformMat4("u_mvp", mvp);
	
	GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m->coreMesh->getVertexCount(), GL_UNSIGNED_INT, nullptr, m_particleCount));
}

void ParticleEffect::loadSettings(boost::property_tree::ptree& configuration) {
	m_cfg = configuration;

	using namespace Renderer;
	setParticleCount(configuration.get<int>("ParticleCount"));

	m_lineEnable = configuration.get<bool>("Line.Enable");
	
	m_mouseGravityEnable = configuration.get<bool>("MouseGravity.Enable");
	m_mouseGravityStrength = 0.f;
	if (m_mouseGravityEnable) {
		m_mouseGravityStrength = configuration.get<float>("MouseGravity.Strength");
	}

	m_lineShader->bind();
	m_lineShader->setUniform1f("u_lineWidth", configuration.get<float>("Line.Width"));
	m_lineShader->setUniform1f("u_maxLineDistance", configuration.get<float>("Line.MaxDistance"));
}
void ParticleEffect::setParticleCount(const int particleCount) {
	using namespace Renderer;

	// particle settings
	const auto sizeMin = m_cfg.get<float>("Particle.Size.Min");
	const auto sizeMax = m_cfg.get<float>("Particle.Size.Max");

	const auto glowEnable = m_cfg.get<bool>("Particle.Glow.Enable");
	float glowSizeMin = 0.f;
	float glowSizeMax = 0.f;
	float glowAmountMin = 0.f;
	float glowAmountMax = 0.f;
	if (glowEnable) {
		glowSizeMin = m_cfg.get<float>("Particle.Glow.Size.Min");
		glowSizeMax = m_cfg.get<float>("Particle.Glow.Size.Max");
		glowAmountMin = m_cfg.get<float>("Particle.Glow.Amount.Min");
		glowAmountMax = m_cfg.get<float>("Particle.Glow.Amount.Max");
	}

	const auto velocityMinX = m_cfg.get<float>("Particle.Velocity.X.Min");
	const auto velocityMaxX = m_cfg.get<float>("Particle.Velocity.X.Max");
	const auto velocityMinY = m_cfg.get<float>("Particle.Velocity.Y.Min");
	const auto velocityMaxY = m_cfg.get<float>("Particle.Velocity.Y.Max");
	const auto wMin = m_cfg.get<float>("Particle.Velocity.Angle.Min");
	const auto wMax = m_cfg.get<float>("Particle.Velocity.Angle.Max");

	auto** particles = new Particle*[particleCount];
	auto* vData = new float[(particleCount + 1) * 3];
	auto* cData = new float[(particleCount + 1) * 3];

	for (int i = 0; i < particleCount; i++) {
		Particle* p;
		if (i < m_particleCount) {
			// we can copy the old one
			p = m_particles[i];
		}
		else {
			// create a new one
			p = createParticle(
				sizeMin, sizeMax,
				glowSizeMin, glowSizeMax, glowAmountMin, glowAmountMax,
				velocityMinX, velocityMaxX, velocityMinY, velocityMaxY,
				wMin, wMax);
		}

		particles[i] = p;
		// buffer constant vector data.
		// float radius, float glowSize, float glowAmount, glm::vec3 color;
		vData[i * 3] = p->radius;
		vData[i * 3 + 1] = p->glowSize;
		vData[i * 3 + 2] = p->glowAmount;
		cData[i * 3] = p->color.r;
		cData[i * 3 + 1] = p->color.g;
		cData[i * 3 + 2] = p->color.b;
	}

	// append mouse as particle
	vData[particleCount * 3] = 0;
	vData[particleCount * 3 + 1] = 0;
	vData[particleCount * 3 + 2] = 0;
	cData[particleCount * 3] = 0.f;
	cData[particleCount * 3 + 1] = 0.f;
	cData[particleCount * 3 + 2] = 0.f;

	delete m_particlesVariablesBuffer;
	m_particlesVariablesBuffer = new TextureBuffer(vData, (particleCount + 1) * 3, TextureBuffer::VEC3);

	delete m_particlesColorsBuffer;
	m_particlesColorsBuffer = new TextureBuffer(cData, (particleCount + 1) * 3, TextureBuffer::VEC3);

	delete m_particlesPositionsBuffer;
	m_particlesPositionsBuffer = new TextureBuffer(nullptr, (particleCount + 1) * 3, TextureBuffer::VEC3);

	delete[] vData;
	delete[] cData;

	for (int i = particleCount; i < m_particleCount; i++) {
		delete m_particles[i];
	}
	
	delete[] m_particles;
	m_particles = particles;

	m_particleCount = particleCount;

	const unsigned int variablesSlot = m_particlesVariablesBuffer->getSlot();
	const unsigned int colorsSlot = m_particlesColorsBuffer->getSlot();
	const unsigned int positionsSlot = m_particlesPositionsBuffer->getSlot();

	m_particlesVariablesBuffer->bind();
	m_particlesColorsBuffer->bind();
	m_particlesPositionsBuffer->bind();

	m_lineShader->bind();
	m_lineShader->setUniform1i("u_pCount", m_particleCount + 1);

	m_lineShader->setUniform1i("u_colors", colorsSlot);
	m_lineShader->setUniform1i("u_positions", positionsSlot);

	m_coreShader->bind();
	m_coreShader->setUniform1i("u_variables", variablesSlot);
	m_coreShader->setUniform1i("u_colors", colorsSlot);
	m_coreShader->setUniform1i("u_positions", positionsSlot);
}

#if SHOW_EDITOR
void ParticleEffect::setFixedSize(const float size) {
	// rescale particle
	m_cfg.put("Particle.Size.Min", size);
	m_cfg.put("Particle.Size.Max", size);

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->radius = size;

		// write sizes to texture buffer
		ptr[i * 3] = m_particles[i]->radius;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}
void ParticleEffect::setSize(const float sizeMin, const float sizeMax) {
	// rescale particle
	const auto oldSizeMin = m_cfg.get<float>("Particle.Size.Min");
	const auto oldSizeMax = m_cfg.get<float>("Particle.Size.Max");
	const float ratio = (sizeMax - sizeMin) / (oldSizeMax - oldSizeMin);

	m_cfg.put("Particle.Size.Min", sizeMin);
	m_cfg.put("Particle.Size.Max", sizeMax);
	
	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->radius = sizeMin + (m_particles[i]->radius - oldSizeMin) * ratio;

		// write sizes to texture buffer
		ptr[i * 3] = m_particles[i]->radius;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}
void ParticleEffect::reInitSizes() const {
	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	const auto min = m_cfg.get<float>("Particle.Size.Min");
	const auto max = m_cfg.get<float>("Particle.Size.Max");
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->radius = rnd(min, max);
		ptr[i * 3] = m_particles[i]->radius;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}

void ParticleEffect::setVelocity(const float velMin, const float velMax) {
	// rescale particle velocity
	const auto fOldVelMinX = m_cfg.get<float>("Particle.Velocity.X.Min");
	const auto fOldVelMaxX = m_cfg.get<float>("Particle.Velocity.X.Max");
	const auto fOldVelMinY = m_cfg.get<float>("Particle.Velocity.Y.Min");
	const auto fOldVelMaxY = m_cfg.get<float>("Particle.Velocity.Y.Max");
	const float ratioX = (velMax - velMin) / (fOldVelMaxX - fOldVelMinX);
	const float ratioY = (velMax - velMin) / (fOldVelMaxY - fOldVelMinY);

	const glm::vec2 oldVelMin = glm::vec2(fOldVelMinX, fOldVelMinY);

	m_cfg.put("Particle.Velocity.X.Min", velMin);
	m_cfg.put("Particle.Velocity.X.Max", velMax);
	m_cfg.put("Particle.Velocity.Y.Min", velMin);
	m_cfg.put("Particle.Velocity.Y.Max", velMax);

	for (int i = 0; i < m_particleCount; i++) {
		glm::vec2 sVel = m_particles[i]->velocity - oldVelMin;
		sVel.x *= ratioX; 
		sVel.y *= ratioY;
		m_particles[i]->velocity = velMin + sVel;
		if (m_particles[i]->velocity.x == 0 && m_particles[i]->velocity.y == 0) {
			m_particles[i]->velocityMagnitude = 0;
		}
		else {
			m_particles[i]->velocityMagnitude = length(m_particles[i]->velocity);
		}
	}
}
void ParticleEffect::setVelocityX(const float velMin, const float velMax) {
	const auto oldVelMin = m_cfg.get<float>("Particle.Velocity.X.Min");
	const auto oldVelMax = m_cfg.get<float>("Particle.Velocity.X.Max");
	const float ratio = (velMax - velMin) / (oldVelMax - oldVelMin);

	m_cfg.put("Particle.Velocity.X.Min", velMin);
	m_cfg.put("Particle.Velocity.X.Max", velMax);

	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->velocity.x = velMin + (m_particles[i]->radius - oldVelMin) * ratio;
		if (m_particles[i]->velocity.x == 0 && m_particles[i]->velocity.y == 0) {
			m_particles[i]->velocityMagnitude = 0;
		}
		else {
			m_particles[i]->velocityMagnitude = length(m_particles[i]->velocity);
		}
	}
}
void ParticleEffect::setVelocityY(const float velMin, const float velMax) {
	const auto oldVelMin = m_cfg.get<float>("Particle.Velocity.Y.Min");
	const auto oldVelMax = m_cfg.get<float>("Particle.Velocity.Y.Max");
	const float ratio = (velMax - velMin) / (oldVelMax - oldVelMin);

	m_cfg.put("Particle.Velocity.Y.Min", velMin);
	m_cfg.put("Particle.Velocity.Y.Max", velMax);

	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->velocity.y = velMin + (m_particles[i]->radius - oldVelMin) * ratio;
		if (m_particles[i]->velocity.x == 0 && m_particles[i]->velocity.y == 0) {
			m_particles[i]->velocityMagnitude = 0;
		}
		else {
			m_particles[i]->velocityMagnitude = length(m_particles[i]->velocity);
		}
	}
}
void ParticleEffect::reInitVelocities() const {
	const auto velMinX = m_cfg.get<float>("Particle.Velocity.X.Min");
	const auto velMaxX = m_cfg.get<float>("Particle.Velocity.X.Max");
	const auto velMinY = m_cfg.get<float>("Particle.Velocity.Y.Min");
	const auto velMaxY = m_cfg.get<float>("Particle.Velocity.Y.Max");
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->velocity = glm::vec2(rnd(velMinX, velMaxX), rnd(velMinY, velMaxY));
		if (m_particles[i]->velocity.x == 0 && m_particles[i]->velocity.y == 0) {
			m_particles[i]->velocityMagnitude = 0;
		}
		else {
			m_particles[i]->velocityMagnitude = length(m_particles[i]->velocity);
		}
	}
}

void ParticleEffect::setAngleVelocity(const float velMin, const float velMax) {
	const auto oldVelMin = m_cfg.get<float>("Particle.Velocity.Angle.Min");
	const auto oldVelMax = m_cfg.get<float>("Particle.Velocity.Angle.Max");
	const float ratio = (velMax - velMin) / (oldVelMax - oldVelMin);

	m_cfg.put("Particle.Velocity.Angle.Min", velMin);
	m_cfg.put("Particle.Velocity.Angle.Max", velMax);

	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->angularVelocity = velMin + (m_particles[i]->angularVelocity - oldVelMin) * ratio;
	}
}
void ParticleEffect::reInitAngleVelocity() const {
	const auto angleVelMin = m_cfg.get<float>("Particle.Velocity.Angle.Min");
	const auto angleVelMax = m_cfg.get<float>("Particle.Velocity.Angle.Max");
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->angularVelocity = rnd(angleVelMin, angleVelMax);
		if (m_particles[i]->velocity.x == 0 && m_particles[i]->velocity.y == 0) {
			m_particles[i]->velocityMagnitude = 0;
		}
		else {
			m_particles[i]->velocityMagnitude = length(m_particles[i]->velocity);
		}
	}
}

void ParticleEffect::setGlowEnable(const bool enable) const {
	const auto sizeMin = m_cfg.get<float>("Particle.Glow.Size.Min");
	const auto sizeMax = m_cfg.get<float>("Particle.Glow.Size.Max");
	const auto amountMin = m_cfg.get<float>("Particle.Glow.Amount.Min");
	const auto amountMax = m_cfg.get<float>("Particle.Glow.Amount.Max");

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		if (enable) {
			m_particles[i]->glowSize = rnd(sizeMin, sizeMax);
			m_particles[i]->glowAmount = rnd(amountMin, amountMax);
		}
		else {
			m_particles[i]->glowSize = 0.f;
			m_particles[i]->glowAmount = 0.f;
		}
		ptr[i * 3 + 1] = m_particles[i]->glowSize;
		ptr[i * 3 + 2] = m_particles[i]->glowAmount;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}

void ParticleEffect::setGlowSize(const float glowSizeMin, const float glowSizeMax) {
	const auto oldSizeMin = m_cfg.get<float>("Particle.Glow.Size.Min");
	const auto oldSizeMax = m_cfg.get<float>("Particle.Glow.Size.Max");
	const float ratio = (glowSizeMax - glowSizeMin) / (oldSizeMax - oldSizeMin);

	m_cfg.put("Particle.Glow.Size.Min", glowSizeMin);
	m_cfg.put("Particle.Glow.Size.Max", glowSizeMax);

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->glowSize = glowSizeMin + (m_particles[i]->glowSize - oldSizeMin) * ratio;
		ptr[i * 3 + 1] = m_particles[i]->glowSize;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}
void ParticleEffect::reInitGlowSize() const {
	const auto sizeMin = m_cfg.get<float>("Particle.Glow.Size.Min");
	const auto sizeMax = m_cfg.get<float>("Particle.Glow.Size.Max");

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->glowSize = rnd(sizeMin, sizeMax);
		ptr[i * 3 + 1] = m_particles[i]->glowSize;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}

void ParticleEffect::setGlowAmount(const float glowAmountMin, const float glowAmountMax) {
	const auto oldAmountMin = m_cfg.get<float>("Particle.Glow.Amount.Min");
	const auto oldAmountMax = m_cfg.get<float>("Particle.Glow.Amount.Max");
	const float ratio = (glowAmountMax - glowAmountMin) / (oldAmountMax - oldAmountMin);

	m_cfg.put("Particle.Glow.Amount.Min", glowAmountMin);
	m_cfg.put("Particle.Glow.Amount.Max", glowAmountMax);

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->glowAmount = glowAmountMin + (m_particles[i]->glowAmount - oldAmountMin) * ratio;
		ptr[i * 3 + 2] = m_particles[i]->glowAmount;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}
void ParticleEffect::reInitGlowAmount() const {
	const auto amountMin = m_cfg.get<float>("Particle.Glow.Amount.Min");
	const auto amountMax = m_cfg.get<float>("Particle.Glow.Amount.Max");

	float* ptr = m_particlesVariablesBuffer->getDataPointer();
	for (int i = 0; i < m_particleCount; i++) {
		m_particles[i]->glowAmount = rnd(amountMin, amountMax);
		ptr[i * 3 + 2] = m_particles[i]->glowAmount;
	}
	m_particlesVariablesBuffer->freeDataPointer();
}

void ParticleEffect::setLineEnable(const bool lineEnable) {
	m_lineEnable = lineEnable;
}
void ParticleEffect::setLineWidth(const float lineWidth) const {
	m_lineShader->bind();
	m_lineShader->setUniform1f("u_lineWidth", lineWidth);
}
void ParticleEffect::setLineMaxDistance(const float lineMaxDistance) const {
	m_lineShader->bind();
	m_lineShader->setUniform1f("u_maxLineDistance", lineMaxDistance);
}

void ParticleEffect::setMouseGravityEnable(const bool mouseGravityEnable) {
	m_mouseGravityEnable = mouseGravityEnable;
	m_mouseGravityStrength = m_cfg.get<float>("MouseGravity.Strength");
}
void ParticleEffect::setMouseGravityStrength(const float mouseGravityStrength) {
	m_mouseGravityStrength = mouseGravityStrength;
	m_cfg.put("MouseGravity.Strength", mouseGravityStrength);
}
#endif

Particle* ParticleEffect::createParticle(
	const float sizeMin, const float sizeMax,
	const float glowSizeMin, const float glowSizeMax, const float glowAmountMin, const float glowAmountMax,
	const float velocityMinX, const float velocityMaxX,
	const float velocityMinY, const float velocityMaxY,
	const float wMin, const float wMax) 
{
	return new Particle(
		// size
		rnd(sizeMin, sizeMax),
		// glow
		rnd(glowSizeMin, glowSizeMax), rnd(glowAmountMin, glowAmountMax),
		// color
		glm::vec3(rnd(0.f, 1.f), rnd(0.f, 1.f), rnd(0.f, 1.f)),
		// position
		glm::vec2(rnd(-1.f, 1.f), rnd(-1.f, 1.f)),
		// velocity
		glm::vec2(rnd(velocityMinX, velocityMaxX), rnd(velocityMinY, velocityMaxY)),
		// angle
		rnd(0.f, 360.f), rnd(wMin, wMax)
	);
}
