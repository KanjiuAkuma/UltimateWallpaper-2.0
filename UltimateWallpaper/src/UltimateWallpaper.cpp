#include "UltimateWallpaper.h"

#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <boost/format.hpp>

#include "util/audioAnalyzer/SmoothSpectrumPreProcessor.h"

UltimateWallpaper::UltimateWallpaper(boost::property_tree::ptree& configuration) {
	m_fpsCounterEnable = configuration.get<bool>("Wallpaper.FpsCounter.Enable");
	m_fpsCounterPosX = configuration.get<float>("Wallpaper.FpsCounter.Position.X");
	m_fpsCounterPosY = configuration.get<float>("Wallpaper.FpsCounter.Position.Y");

	m_audioPreProcessor = new SmoothSpectrumPreProcessor(
		configuration.get<float>("Wallpaper.AudioResponse.SmoothingFactor"), 
		configuration.get<float>("Wallpaper.AudioResponse.Threshold"));
	if (configuration.get<bool>("Wallpaper.Slideshow.Enable")) {
		m_slideShow = new SlideShow(m_audioPreProcessor->getSpectrum());
		m_slideShow->loadSettings(configuration.get_child("Wallpaper.Slideshow"));
	}
	
	if (configuration.get<bool>("Wallpaper.ParticleEffect.Enable")) {
		m_particleEffect = new ParticleEffect(m_audioPreProcessor->getSpectrum());
		m_particleEffect->loadSettings(configuration.get_child("Wallpaper.ParticleEffect"));
	}

	static boost::format fmt("Wallpaper.Equalizer.Eq_%d");
	const int equalizerCount = configuration.get<int>("Wallpaper.Equalizer.Count");
	for (int i = 0; i < equalizerCount; i++) {
		auto* eq = new Equalizer(m_audioPreProcessor->getSpectrum());
		eq->loadSettings(configuration.get_child((fmt % i).str()));
		m_equalizers.push_back(eq);
	}

	// if (configuration.get<bool>("Wallpaper.Equalizer.Enable")) {
	// 	m_equalizer = new Equalizer(m_audioPreProcessor->getSpectrum());
	// 	m_equalizer->loadSettings(configuration.get_child("Wallpaper.Equalizer"));
	// }
	

	/* open gl setup */
	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

UltimateWallpaper::~UltimateWallpaper() {
	delete m_slideShow;
	delete m_particleEffect;
	delete m_audioPreProcessor;

	for (auto& m_equalizer : m_equalizers) {
		delete m_equalizer;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UltimateWallpaper::render() {
	m_audioStreamReader->updateSpectrum();
	m_audioPreProcessor->updateSpectrum();
	float w = m_sceneWidth;
	float h = m_sceneHeight;

	#if WP_DEBUG | WP_RELEASE
	w *= 1.1f;
	h *= 1.1f;
	#endif

	if (m_slideShow) m_slideShow->render(glm::ortho(-w, w, -h, h, -1.f, 1.f));

	w = m_windowWidth / m_windowHeight;
	h = 1.f;

	#if WP_DEBUG | WP_RELEASE
	w *= 1.1f;
	h *= 1.1f;
	#endif

	glm::mat4 proj = glm::ortho(-w, w, -h, h, -1.f, 1.f);

	if (!m_equalizers.empty()) {
		for (auto& m_equalizer : m_equalizers) {
			m_equalizer->render(proj);
		}
	}
	if (m_fpsCounterEnable) renderFpsCounter(true);

	double dMouseX, dMouseY;
	glfwGetCursorPos(m_window, &dMouseX, &dMouseY);

	const glm::vec2 mousePos = glm::vec2(
		float(w * (-1. + 2. * dMouseX / m_windowWidth)),
		float(-h * (-1. + 2. * dMouseY / m_windowHeight))
	);

	if (m_particleEffect) m_particleEffect->render(proj, mousePos);
}

void UltimateWallpaper::update(const float dt) {
	double dMouseX, dMouseY;
	glfwGetCursorPos(m_window, &dMouseX, &dMouseY);

	const float w = m_windowWidth / m_windowHeight;
	const float h = 1.f;
	const glm::vec2 mousePos = glm::vec2(
		float(w * (-1. + 2. * dMouseX / m_windowWidth)),
		float(-h * (-1. + 2. * dMouseY / m_windowHeight))
	);

	if (m_slideShow) m_slideShow->update(dt);
	if (!m_equalizers.empty()) {
		for (auto& m_equalizer : m_equalizers) {
			m_equalizer->update(dt);
		}
	}
	if (m_particleEffect) m_particleEffect->update(dt, mousePos, w, h);
}

void UltimateWallpaper::renderFpsCounter(const bool newFrame) const {
	if (newFrame) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));
	ImGui::SetNextWindowPos(ImVec2(m_fpsCounterPosX, m_fpsCounterPosY));
	ImGui::Begin("FpsCount", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
	ImGui::Text("%.1f FPS (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
	ImGui::End();

	if (newFrame) {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
