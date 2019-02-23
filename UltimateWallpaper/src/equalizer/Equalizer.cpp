#include "Equalizer.h"

Equalizer::Equalizer(Spectrum* spectrum) : m_audioSpectrum(spectrum) {
	using namespace JApp::Renderer;

	float vertices[8]
	{
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,
	};

	auto* vb = new VertexBuffer(vertices, 8 * sizeof(float));
	auto* vbl = new VertexBufferLayout();
	vbl->push<float>(2);
	auto* va = new VertexArray();
	va->addBuffer(vb, vbl);

	unsigned int indices[]
	{
		0, 1, 2,
		2, 3, 0,
	};
	auto* ib = new IndexBuffer(indices, 6);
	
	m_mesh = new Mesh(va, vb, vbl, ib);
}

Equalizer::~Equalizer() {
	delete m_mesh;
	delete m_shader;
}

void Equalizer::update(const float dt) {
	if (m_colorFrequency != 0) {
		m_colorTime = glm::mod(m_colorTime + dt * m_colorFrequency, 1.f / m_colorFrequency);
	}
	if (m_rotationFrequency != 0) {
		m_rotationTime = glm::mod(m_rotationTime + dt * m_rotationFrequency, 1.f / m_rotationFrequency);
	}
}

void Equalizer::render(const glm::mat4 mvp) const {
	m_mesh->bind();
	m_shader->bind();

	float* ptr = m_spectrumBuffer->getDataPointer();
	const float nyquist = m_audioSpectrum->nyquist;
	for (int i = 0; i < m_bars; i++) {
		const float f0 = float(1. - log10(m_bars + 1 - i) / log10(m_bars + 1)) * nyquist;
		const float f1 = float(1. - log10(m_bars - i) / log10(m_bars + 1)) * nyquist;
		const float v = log(1.f + m_audioSpectrum->sumRange(f0, f1) * m_baseAmplifier) * m_peakAmplifier;
		ptr[i] = v;
	}
	m_spectrumBuffer->freeDataPointer();
	m_shader->setUniformMat4("u_mvp", mvp * m_model);
	m_shader->setUniform1f("u_colorValue", glm::mod(m_colorOffset + m_colorTime, 1.f));
	m_shader->setUniform1f("u_angleValue", glm::mod(m_rotationTime, 1.f));

	GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_mesh->getVertexCount(), GL_UNSIGNED_INT, nullptr, m_bars));
}

void Equalizer::loadSettings(boost::property_tree::ptree& cfg) {
	// get shader
	delete m_shader;
	delete m_spectrumBuffer;

	const std::string type = cfg.get<std::string>("Type");

	if (type == "Line") {
		m_shader = JApp::Renderer::Shader::fromFiles(
			"resources/shaders/equalizer/lineEqualizerVertex.shader", 
			"resources/shaders/equalizer/lineEqualizerFragment.shader");
		m_shader->bind();

		m_size = glm::vec3(cfg.get<float>("Size.Width"), cfg.get<float>("Size.Height") * (m_flip ? -1.f : 1.f), 1.f);
	}
	else if (type == "Ring") {
		const std::string subType = cfg.get<std::string>("SubType");
		if (subType == "Bar") {
			m_shader = JApp::Renderer::Shader::fromFiles(
				"resources/shaders/equalizer/ringBarEqualizerVertex.shader", 
				"resources/shaders/equalizer/ringEqualizerFragment.shader");
		}
		else if (subType == "Segment") {
			m_shader = JApp::Renderer::Shader::fromFiles(
				"resources/shaders/equalizer/ringSegmentEqualizerVertex.shader", 
				"resources/shaders/equalizer/ringEqualizerFragment.shader");
		}
		else {
			APP_CRITICAL("Unknown equalizer sub type '{}'", subType);
			ASSERT(false);
		}

		// load ring specific settings

		m_shader->bind();
		m_shader->setUniform1f("u_innerRadius", cfg.get<float>("Ring.Radius.Inner"));
		m_shader->setUniform1f("u_outerRadius", cfg.get<float>("Ring.Radius.Outer"));

		m_size = glm::vec3(cfg.get<float>("Size.Width"), cfg.get<float>("Size.Width") * (m_flip ? -1.f : 1.f), 1.f);

		if (cfg.get<bool>("Ring.Rotation.Enable")) {
			m_rotationFrequency = cfg.get<float>("Ring.Rotation.Speed");
		}
		else {
			m_rotationFrequency = 0.f;
		}

		if (cfg.get<bool>("Ring.Rounding.Inner")) {
			m_shader->setUniform1i("u_innerRounding", 1);
		}
		else {
			m_shader->setUniform1i("u_innerRounding", 0);
		}
		if (cfg.get<bool>("Ring.Rounding.Outer")) {
			m_shader->setUniform1i("u_outerRounding", 1);
		}
		else {
			m_shader->setUniform1i("u_outerRounding", 0);
		}
	}
	else {
		APP_CRITICAL("Unknown equalizer type '{}'", type);
		ASSERT(false);
	}
	
	// load mvp
	m_flip = cfg.get<bool>("Flip");
	m_position = glm::vec3(cfg.get<float>("Position.X"), cfg.get<float>("Position.Y"), 0.f);
	m_angle = cfg.get<float>("Position.Angle");
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));

	// load spectrum buffer
	m_bars = cfg.get<int>("BarCount");
	m_shader->setUniform1i("u_bars", m_bars);

	m_spectrumBuffer = new JApp::Renderer::TextureBuffer(nullptr, m_bars, JApp::Renderer::TextureBuffer::FLOAT);
	m_spectrumBuffer->bind();
	m_shader->setUniform1i("u_spectrum", m_spectrumBuffer->getSlot());

	// load all the other settings
	m_colorOffset = cfg.get<float>("Color.Offset");
	m_shader->setUniform1f("u_alpha", cfg.get<float>("Color.Alpha"));
	if (cfg.get<bool>("Color.Flow.Enable")) {
		m_colorFrequency = cfg.get<float>("Color.Flow.Speed");
	}
	else {
		m_colorFrequency = 0.f;
	}

	m_baseAmplifier = cfg.get<float>("BaseAmplifier");
	m_peakAmplifier = cfg.get<float>("PeakAmplifier");

	m_shader->setUniform1i("u_bars", m_bars);
	m_shader->setUniform1f("u_barWidth", cfg.get<float>("BarWidth"));
	m_shader->setUniform1f("u_baseAmplitude", cfg.get<float>("BaseAmplitude"));
}

#if SHOW_EDITOR
void Equalizer::setBarCount(const int barCount) {
	m_bars = barCount;


	delete m_spectrumBuffer;
	m_spectrumBuffer = new JApp::Renderer::TextureBuffer(nullptr, m_bars, JApp::Renderer::TextureBuffer::FLOAT);
	m_spectrumBuffer->bind();

	m_shader->bind();
	m_shader->setUniform1i("u_bars", m_bars);
	m_shader->setUniform1i("u_spectrum", m_spectrumBuffer->getSlot());
}

void Equalizer::setBarWidth(const float barWidth) const {
	m_shader->bind();
	m_shader->setUniform1f("u_barWidth", barWidth);
}

void Equalizer::setBaseAmplitude(const float baseAmplitude) const {
	m_shader->bind();
	m_shader->setUniform1f("u_baseAmplitude", baseAmplitude);
}

void Equalizer::setBaseAmplifier(const float baseAmplifier) {
	m_baseAmplifier = baseAmplifier;
}

void Equalizer::setPeakAmplifier(const float peakAmplifier) {
	m_peakAmplifier = peakAmplifier;
}

void Equalizer::setColorOffset(const float offset) {
	m_colorOffset = offset;
}

void Equalizer::setAlpha(const float alpha) const {
	m_shader->bind();
	m_shader->setUniform1f("u_alpha", alpha);
}

void Equalizer::disableColorFlow() {
	m_colorFrequency = 0.f;
}

void Equalizer::setColorFlowSpeed(const float speed) {
	m_colorFrequency = speed;
}

void Equalizer::setPositionX(const float x) {
	m_position.x = x;
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setPositionY(const float y) {
	m_position.y = y;
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setAngle(const float degrees) {
	m_angle = degrees;
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setFlip(const bool flip) {
	m_flip = flip;
	if (flip) {
		if (0.f < m_size.y) {
			m_size.y *= -1.f;
		}
	}
	else {
		if (m_size.y < 0.f) {
			m_size.y *= -1.f;
		}
	}
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setWidth(const float width) {
	m_size.x = width;
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setHeight(const float height) {
	m_size.y = height * (m_flip ? -1.f : 1.f);
	m_model = rotate(scale(translate(glm::mat4(1.f), m_position), m_size), glm::radians(m_angle), glm::vec3(0.f, 0.f, 1.f));
}

void Equalizer::setInnerRadius(const float innerRadius) const {
	m_shader->bind();
	m_shader->setUniform1f("u_innerRadius", innerRadius);
}

void Equalizer::setOuterRadius(const float outerRadius) const {
	m_shader->bind();
	m_shader->setUniform1f("u_outerRadius", outerRadius);
}

void Equalizer::setInnerRounding(const bool enable) const {
	m_shader->bind();
	m_shader->setUniform1i("u_innerRounding", enable ? 1 : 0);
}

void Equalizer::setOuterRounding(const bool enable) const {
	m_shader->bind();
	m_shader->setUniform1i("u_outerRounding", enable ? 1 : 0);
}

void Equalizer::disableRotation() {
	m_rotationFrequency = 0.f;
}

void Equalizer::setRotationSpeed(const float speed) {
	m_rotationFrequency = speed;
}
#endif
