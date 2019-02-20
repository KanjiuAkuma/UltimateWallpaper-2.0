#if SHOW_EDITOR

#include "Editor.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>

Editor::Editor(boost::property_tree::ptree& configuration) : UltimateWallpaper(configuration), m_cfg(configuration) {
	m_fpsCounterEnableOverride = m_fpsCounterEnable;
	m_fpsCounterEnable = false;
}

Editor::~Editor() = default;

void Editor::render() {
	float speedMod = 1.f;
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
		speedMod = .5f;
	}
	
	// render wallpaper
	UltimateWallpaper::render();

	// add ui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_fpsCounterEnableOverride) {
		renderFpsCounter(false);
	}

	static ImVec2 wpos = ImVec2(m_windowWidth, m_windowHeight);
	static bool clip = false;

	ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 0.f), ImVec2(m_windowWidth, m_windowHeight - 200));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	if (clip) {
		ImGui::SetNextWindowPos(wpos);
		clip = false;
	}
	ImGui::Begin(std::string("Configuration (" + std::string(m_currentName) + ")").c_str(), 0, ImGuiWindowFlags_MenuBar);

	static bool showLoad = false;
	static bool showSave = false;
	static char loadName[128];
	static char saveName[128];

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Apply")) {
				saveSettings("cfg");
			}
			
			if (ImGui::MenuItem("Load")) {
				showLoad = true;
			}
			
			if (ImGui::MenuItem("Save")) {
				saveSettings(m_currentName);
			}

			if (ImGui::MenuItem("Save as")) {
				showSave = true;
			}

			if (ImGui::MenuItem("Reset")) {
				loadSettings(m_currentName);
			}
			if (ImGui::MenuItem("Restore default")) {
				loadSettings("default-cfg");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	static bool isFirstShowLoad = true;
	if (showLoad) {
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowPos(ImVec2(m_windowWidth / 2, m_windowHeight / 2));
		ImGui::Begin("Load", 0, ImGuiWindowFlags_NoDecoration);
		bool load = false;
		load |= ImGui::InputText("Name", loadName, 128, ImGuiInputTextFlags_EnterReturnsTrue);
		if (isFirstShowLoad) {
			ImGui::SetKeyboardFocusHere();
			isFirstShowLoad = false;
		}

		load |= ImGui::Button("Load");
		
		if (load) {
			try {
				loadSettings(loadName);
				m_currentName = loadName;
				showLoad = false;
			} catch (boost::exception&) {}
			isFirstShowLoad = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			showLoad = false;
			isFirstShowLoad = true;
		}
		ImGui::End();
	}

	static bool isFirstShowSave = true;
	if (showSave) {
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowPos(ImVec2(m_windowWidth / 2, m_windowHeight / 2));
		ImGui::Begin("Save as", 0, ImGuiWindowFlags_NoDecoration);
		bool save = false;
		save |= ImGui::InputText("Name", saveName, 128, ImGuiInputTextFlags_EnterReturnsTrue);
		if (isFirstShowSave) {
			ImGui::SetKeyboardFocusHere();
			isFirstShowSave = false;
		}
		save |= ImGui::Button("Save");
		
		if (save) {
			m_currentName = saveName;
			saveSettings(saveName);
			showSave = false;
			isFirstShowSave = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			showSave = false;
			isFirstShowSave = true;
		}
		ImGui::End();
	}

	if (ImGui::CollapsingHeader("Fps counter")) {
		if (ImGui::Checkbox("Enable", &m_fpsCounterEnableOverride)) {
			m_cfg.put("Wallpaper.FpsCounter.Enable", m_fpsCounterEnableOverride);
		}

		if (ImGui::DragFloat("Position X", &m_fpsCounterPosX, 1.f * speedMod, 0.f, m_windowWidth)) {
			m_cfg.put("Wallpaper.FpsCounter.Position.X", m_fpsCounterPosX);
		}

		if (ImGui::DragFloat("Position Y", &m_fpsCounterPosY, 1.f * speedMod, 0.f, m_windowHeight)) {
			m_cfg.put("Wallpaper.FpsCounter.Position.Y", m_fpsCounterPosY);
		}
	}

	if (ImGui::CollapsingHeader("Audio response")) {
		auto smoothingFactor = m_cfg.get<float>("Wallpaper.AudioResponse.SmoothingFactor");
		if (ImGui::DragFloat("Smoothing", &smoothingFactor, 10.f * speedMod, 0.f, 500.f)) {
			m_audioPreProcessor->setSmoothingFactor(smoothingFactor);
			m_cfg.put("Wallpaper.AudioResponse.SmoothingFactor", smoothingFactor);
		}

		auto threshold = m_cfg.get<float>("Wallpaper.AudioResponse.Threshold");
		if (ImGui::DragFloat("Threshold", &threshold, 0.1f * speedMod, 0.f, 5.f)) {
			m_audioPreProcessor->setThreshold(threshold);
			m_cfg.put("Wallpaper.AudioResponse.Threshold", threshold);
		}
	}

	if (ImGui::CollapsingHeader("Slideshow")) {
		renderSlideShowSettings(speedMod);
	}

	if (ImGui::CollapsingHeader("Equalizer")) {
		renderEqualizerSettings(speedMod);
	}

	if (ImGui::CollapsingHeader("Particle Effect")) {
		renderParticleEffectSettings(speedMod);
	}

	// clipping
	const float clipRange = 30.f;
	wpos = ImGui::GetWindowPos();
	ImVec2 wsize = ImGui::GetWindowSize();

	if (!ImGui::IsMouseDown(0)) {
		if (wpos.x - clipRange < 0.f) {
			wpos.x = 0.f;
			clip = true;
		}
		else if (m_windowWidth - wsize.x - clipRange < wpos.x) {
			wpos.x = m_windowWidth - wsize.x;
			clip = true;
		}

		if (wpos.y - clipRange < 0.f) {
			wpos.y = 0.f;
			clip = true;
		}
		else if (m_windowHeight - wsize.y - clipRange < wpos.y) {
			wpos.y = m_windowHeight - wsize.y;
			clip = true;
		}
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::renderSlideShowSettings(const float speedMod) {
	bool enable = m_cfg.get<bool>("Wallpaper.Slideshow.Enable");
	if (ImGui::Checkbox("Slideshow enable", &enable)) {
		if (enable) {
			delete m_slideShow;
			m_slideShow = new SlideShow(m_audioPreProcessor->getSpectrum());
			m_slideShow->loadSettings(m_cfg.get_child("Wallpaper.Slideshow"));
		}
		else {
			delete m_slideShow;
			m_slideShow = nullptr;
		}
		m_cfg.put("Wallpaper.Slideshow.Enable", enable);
	}

	if (enable) {
		static char imgDir[1024];
		static bool firstTime = true, dirValid = true, colorChanged = false;
		if (firstTime) {
			firstTime = false;
			auto activeImgDir = m_cfg.get<std::string>("Wallpaper.Slideshow.ImageDirectory");
			memcpy(imgDir, activeImgDir.c_str(), activeImgDir.size() * sizeof(char));
		}

		if (!dirValid) {
			colorChanged = true;
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
		}
		if (ImGui::InputText("Image directory", imgDir, IM_ARRAYSIZE(imgDir), ImGuiInputTextFlags_EnterReturnsTrue)) {
			const std::string sImgDir = std::string(imgDir);
			std::string searchPath = sImgDir + "/*.*";

			WIN32_FIND_DATA fd;
			HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &fd);
			dirValid = hFind != INVALID_HANDLE_VALUE;
			if (dirValid) {
				m_slideShow->setImageDirectory(sImgDir);
				m_cfg.put("Wallpaper.Slideshow.ImageDirectory", sImgDir);
			}
		}
		if (colorChanged) {
			colorChanged = false;
			ImGui::PopStyleColor();
		}

		static int durBuf[3];
		int dur = m_cfg.get<int>("Wallpaper.Slideshow.ImageDuration");
		durBuf[2] = dur % 60;
		dur /= 60;
		durBuf[1] = dur % 60;
		durBuf[0] = dur / 60;
		bool durChanged = false;
		durChanged |= ImGui::DragInt3("Image duration (hh:mm:ss)", &durBuf[0], 1, 0, 59, "%d");

		if (ImGui::TreeNode("Transition")) {
			static char* transitions[]{ "Alpha", "Not Alpha" };
			int selection = 0;
			const std::string tranType = m_cfg.get<std::string>("Wallpaper.Slideshow.Transition.Type");
			if (tranType != "Alpha") {
				selection = 1;
			}

			if (ImGui::Combo("Type", &selection, transitions, 2)) {
				APP_INFO("Changed transition type to {}", transitions[selection]);
			}

			auto tDur = m_cfg.get<float>("Wallpaper.Slideshow.Transition.Duration");
			if (ImGui::DragFloat("Duration", &tDur, speedMod, 0.1f, 100.f)) {
				m_slideShow->setTransitionDuration(tDur);
				m_cfg.put("Wallpaper.Slideshow.Transition.Duration", tDur);
			}
			ImGui::TreePop();
		}

		if (durChanged) {
			const int durNew = durBuf[2] + durBuf[1] * 60 + durBuf[0] * 3600;
			m_slideShow->setImageDuration(durNew);
			m_cfg.put("Wallpaper.Slideshow.ImageDuration", durNew);
		}

		bool audioResponseEnable = m_cfg.get<bool>("Wallpaper.Slideshow.AudioResponse.Enable");
		if (ImGui::Checkbox("Slideshow audio response", &audioResponseEnable)) {
			m_slideShow->setAudioResponseEnable(audioResponseEnable);
			m_cfg.put("Wallpaper.Slideshow.AudioResponse.Enable", audioResponseEnable);
		}

		if (audioResponseEnable) {
			auto bassPeakAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.BassPeakAmplifier");
			if (ImGui::DragFloat("Bass peak", &bassPeakAmplifier, .1f * speedMod, 0.f, 10.f)) {
				m_slideShow->setBassPeakAmplifier(bassPeakAmplifier);
				m_cfg.put("Wallpaper.Slideshow.AudioResponse.BassPeakAmplifier", bassPeakAmplifier);
			}

			auto highPeakAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.HighPeakAmplifier");
			if (ImGui::DragFloat("High peak", &highPeakAmplifier, .1f * speedMod, 0.f, 10.f)) {
				m_slideShow->setHighPeakAmplifier(highPeakAmplifier);
				m_cfg.put("Wallpaper.Slideshow.AudioResponse.HighPeakAmplifier", highPeakAmplifier);
			}

			if (ImGui::TreeNode("Advanced")) {
				auto bassBaseAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.BassBaseAmplifier");
				if (ImGui::DragFloat("Bass base", &bassBaseAmplifier, .1f * speedMod, .01f, 20.f)) {
					m_slideShow->setBassBaseAmplifier(bassBaseAmplifier);
					m_cfg.put("Wallpaper.Slideshow.AudioResponse.BassBaseAmplifier", bassBaseAmplifier);
				}

				auto highBaseAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.HighBaseAmplifier");
				if (ImGui::DragFloat("High base", &highBaseAmplifier, .1f * speedMod, .01f, 20.f)) {
					m_slideShow->setHighBaseAmplifier(highBaseAmplifier);
					m_cfg.put("Wallpaper.Slideshow.AudioResponse.HighBaseAmplifier", highBaseAmplifier);
				}

				auto brightnessFilter = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.BrightnessFilter");
				if (ImGui::DragFloat("Brightness Filter", &brightnessFilter, 1.f * speedMod, 0.f, 10.f)) {
					m_slideShow->setBrightnessFilter(brightnessFilter);
					m_cfg.put("Wallpaper.Slideshow.AudioResponse.BrightnessFilter", brightnessFilter);
				}

				ImGui::TreePop();
			}
		}
	}
}

void Editor::renderEqualizerSettings(const float speedMod) {
	static int count = m_cfg.get<int>("Wallpaper.Equalizer.Count");
	static int currentEq = 0;
	static boost::format cfgFmt("Wallpaper.Equalizer.Eq_%d");

	if (ImGui::Button("Add equalizer")) {
		const std::string id = (cfgFmt % count).str();
		currentEq = count;
		count++;
		m_cfg.put("Wallpaper.Equalizer.Count", count);
		m_cfg.put(id + ".Name", "New Equalizer");
		m_cfg.put(id + ".Type", "Line");
		m_cfg.put(id + ".SubType", "Bar");
		m_cfg.put(id + ".BarCount", 16);
		m_cfg.put(id + ".BarWidth", .9);
		m_cfg.put(id + ".BaseAmplitude", 0);
		m_cfg.put(id + ".BaseAmplifier", 5);
		m_cfg.put(id + ".PeakAmplifier", 1);
		m_cfg.put(id + ".Color.Alpha", .7);
		m_cfg.put(id + ".Color.Offset", 0);
		m_cfg.put(id + ".Color.Flow.Enable", true);
		m_cfg.put(id + ".Color.Flow.Speed", .001);
		m_cfg.put(id + ".Position.X", 0);
		m_cfg.put(id + ".Position.Y", 0);
		m_cfg.put(id + ".Position.Angle", 0);
		m_cfg.put(id + ".Flip", false);
		m_cfg.put(id + ".Size", .5);
		m_cfg.put(id + ".Ring.Radius.Inner", 1);
		m_cfg.put(id + ".Ring.Radius.Outer", 2);
		m_cfg.put(id + ".Ring.Rounding.Inner", true);
		m_cfg.put(id + ".Ring.Rounding.Outer", true);
		m_cfg.put(id + ".Ring.Rotation.Enable", true);
		m_cfg.put(id + ".Ring.Rotation.Speed", .01);

		auto* eq = new Equalizer(m_audioPreProcessor->getSpectrum());
		eq->loadSettings(m_cfg.get_child(id));
		m_equalizers.push_back(eq);
	}

	if (0 < count) {

		if (ImGui::BeginCombo("Current equalizer", m_cfg.get<std::string>((cfgFmt % currentEq).str() + ".Name").c_str())) {
			for (int i = 0; i < count; i++) {
				if (ImGui::Selectable(m_cfg.get<std::string>((cfgFmt % i).str() + ".Name").c_str())) {
					currentEq = i;
				}
				if (i == currentEq) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		std::string id = (cfgFmt % currentEq).str();

		if (ImGui::Button("Copy")) {
			const std::string idOld = id;
			id = (cfgFmt % count).str();
			currentEq = count;
			count++;
			
			m_cfg.put("Wallpaper.Equalizer.Count", count);

			const boost::property_tree::ptree node = m_cfg.get_child(idOld);
			m_cfg.put_child(id, node);
			m_cfg.put(id + ".Name", "Copy of " + m_cfg.get<std::string>(idOld + ".Name"));
			
			auto* eq = new Equalizer(m_audioPreProcessor->getSpectrum());
			eq->loadSettings(m_cfg.get_child(id));
			m_equalizers.push_back(eq);
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove")) {
			m_cfg.erase(id);
			count--;

			for (int i = currentEq; i < count; i++) {
				const std::string idOld = (cfgFmt % (i + 1)).str();
				const std::string idNew = (cfgFmt % i).str();

				const boost::property_tree::ptree node = m_cfg.get_child(idOld);
				m_cfg.put_child(idNew, node);
			}
			
			delete m_equalizers[currentEq];
			m_equalizers.erase(m_equalizers.begin() + currentEq);
			if (0 < currentEq) {
				currentEq--;
			}
		}

		char name[128];
		auto sname = m_cfg.get<std::string>(id + ".Name");
		for (int i = 0; i < 128; i++) {
			if (i < sname.size()) {
				name[i] = sname[i];
			}
			else {
				name[i] = '\0';
			}
		}

		if (ImGui::InputText("Name", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_cfg.put(id + ".Name", name);
		}

		auto currentType = m_cfg.get<std::string>(id + ".Type") == "Line" ? 0 : 1;
		auto currentSubType = m_cfg.get<std::string>(id + ".SubType") == "Bar" ? 0 : 1;
		static char* types[]{ "Line", "Ring" };
		static char* subTypes[]{ "Bar", "Segment" };

		if (ImGui::Combo("Type", &currentType, types, 2)) {
			m_cfg.put(id + ".Type", currentType == 0 ? "Line" : "Ring");
			m_equalizers[currentEq]->loadSettings(m_cfg.get_child(id));
		}

		if (currentType == 1) {
			if (ImGui::Combo("Sub type", &currentSubType, subTypes, 2)) {
				m_cfg.put(id + ".SubType", currentSubType == 0 ? "Bar" : "Segment");
				m_equalizers[currentEq]->loadSettings(m_cfg.get_child(id));
			}
		}

		auto barCount = m_cfg.get<int>(id + ".BarCount");
		if (ImGui::DragInt("Bar count", &barCount, speedMod == 1 ? 1.f : 10.f, 2, 4096)) {
			m_equalizers[currentEq]->setBarCount(barCount);
			m_cfg.put(id + ".BarCount", barCount);
		}

		auto barWidth = m_cfg.get<float>(id + ".BarWidth");
		if (currentType == 0 || currentSubType == 0) {
			if (ImGui::DragFloat("Bar width", &barWidth, .05f * speedMod, .05f, 1.f)) {
				m_equalizers[currentEq]->setBarWidth(barWidth);
				m_cfg.put(id + ".BarWidth", barWidth);
			}
		}

		auto baseAmplitude = m_cfg.get<float>(id + ".BaseAmplitude");
		if (ImGui::DragFloat("Base amplitude", &baseAmplitude, .001f * speedMod, 0.f, 10.f)) {
			m_equalizers[currentEq]->setBaseAmplitude(baseAmplitude);
			m_cfg.put(id + ".BaseAmplitude", baseAmplitude);
		}

		auto baseAmplifier = m_cfg.get<float>(id + ".BaseAmplifier");
		if (ImGui::DragFloat("Base amplifier", &baseAmplifier, .001f * speedMod, 0.f, 10.f)) {
			m_equalizers[currentEq]->setBaseAmplifier(baseAmplifier);
			m_cfg.put(id + ".BaseAmplifier", baseAmplifier);
		}

		auto peakAmplifier = m_cfg.get<float>(id + ".PeakAmplifier");
		if (ImGui::DragFloat("Peak amplifier", &peakAmplifier, .001f * speedMod, 0.f, 10.f)) {
			m_equalizers[currentEq]->setPeakAmplifier(peakAmplifier);
			m_cfg.put(id + ".PeakAmplifier", peakAmplifier);
		}

		ImGui::Text("Color");

		auto alpha = m_cfg.get<float>(id + ".Color.Alpha");
		if (ImGui::DragFloat("Alpha", &alpha, .01f * speedMod, 0.f, 1.f)) {
			m_equalizers[currentEq]->setAlpha(alpha);
			m_cfg.put(id + ".Color.Alpha", alpha);
		}

		auto colorFlowEnable = m_cfg.get<bool>(id + ".Color.Flow.Enable");
		auto colorFlowSpeed = m_cfg.get<float>(id + ".Color.Flow.Speed");

		if (ImGui::Checkbox("Color flow enable", &colorFlowEnable)) {
			if (colorFlowEnable) {
				m_equalizers[currentEq]->setColorFlowSpeed(colorFlowSpeed);

			}
			else {
				m_equalizers[currentEq]->disableColorFlow();
			}
			m_cfg.put(id + ".Color.Flow.Enable", colorFlowEnable);
		}

		if (colorFlowEnable) {
			if (ImGui::DragFloat("Color flow speed", &colorFlowSpeed, .01f * speedMod, -1.f, 1.f)) {
				m_equalizers[currentEq]->setColorFlowSpeed(colorFlowSpeed);
				m_cfg.put(id + ".Color.Flow.Speed", colorFlowSpeed);
			}
		}
		else {
			auto colorOffset = m_cfg.get<float>(id + ".Color.Offset");
			if (ImGui::DragFloat("Color (Hue)", &colorOffset, .005f * speedMod, 0.f, 1.f)) {
				m_equalizers[currentEq]->setColorOffset(colorOffset);
				m_cfg.put(id + ".Color.Offset", colorOffset);
			}
		}

		ImGui::Text("Position");
		auto x = m_cfg.get<float>(id + ".Position.X");
		if (ImGui::DragFloat("X", &x, .01f * speedMod, -100.f, 100.f)) {
			m_equalizers[currentEq]->setPositionX(x);
			m_cfg.put(id + ".Position.X", x);
		}

		auto y = m_cfg.get<float>(id + ".Position.Y");
		if (ImGui::DragFloat("Y", &y, .01f * speedMod, -100.f, 100.f)) {
			m_equalizers[currentEq]->setPositionY(y);
			m_cfg.put(id + ".Position.Y", y);
		}

		auto angle = m_cfg.get<float>(id + ".Position.Angle");
		if (ImGui::DragFloat("Angle", &angle, 10.f * speedMod, -360.f, 360.f)) {
			m_equalizers[currentEq]->setAngle(angle);
			m_cfg.put(id + ".Position.Angle", angle);
		}

		auto size = m_cfg.get<float>(id + ".Size");
		if (ImGui::DragFloat("Size", &size, .1f * speedMod, 0.01f, 10.f)) {
			m_equalizers[currentEq]->setWidth(size);
			if (currentType == 1) {
				m_equalizers[currentEq]->setHeight(size);
			}
			m_cfg.put(id + ".Size", size);
		}

		auto flip = m_cfg.get<bool>(id + ".Flip");
		if (ImGui::Checkbox("Flip", &flip)) {
			m_equalizers[currentEq]->setFlip(flip);
			m_cfg.put(id + ".Flip", flip);
		}

		// ring only
		if (currentType == 1) {
			ImGui::Text("Radius");
			auto innerRadius = m_cfg.get<float>(id + ".Ring.Radius.Inner");
			auto outerRadius = m_cfg.get<float>(id + ".Ring.Radius.Outer");
			if (ImGui::DragFloat("Inner", &innerRadius, 0.01f * speedMod, 0.f, 100.f)) {
				if (outerRadius < innerRadius) {
					m_equalizers[currentEq]->setOuterRadius(innerRadius);
					m_cfg.put(id + ".Ring.Radius.Outer", innerRadius);
				}
				m_equalizers[currentEq]->setInnerRadius(innerRadius);
				m_cfg.put(id + ".Ring.Radius.Inner", innerRadius);
			}

			if (ImGui::DragFloat("Outer", &outerRadius, 0.01f * speedMod, 0.f, 100.f)) {
				if (outerRadius < innerRadius) {
					m_equalizers[currentEq]->setInnerRadius(outerRadius);
					m_cfg.put(id + ".Ring.Radius.Inner", outerRadius);
				}
				m_equalizers[currentEq]->setOuterRadius(outerRadius);
				m_cfg.put(id + ".Ring.Radius.Outer", outerRadius);
			}

			auto innerRounding = m_cfg.get<bool>(id + ".Ring.Rounding.Inner");
			if (ImGui::Checkbox("Inner rounding", &innerRounding)) {
				m_equalizers[currentEq]->setInnerRounding(innerRounding);
				m_cfg.put(id + ".Ring.Rounding.Inner", innerRounding);
			}

			auto outerRounding = m_cfg.get<bool>(id + ".Ring.Rounding.Outer");
			if (ImGui::Checkbox("Outer rounding", &outerRounding)) {
				m_equalizers[currentEq]->setOuterRounding(outerRounding);
				m_cfg.put(id + ".Ring.Rounding.Outer", outerRounding);
			}

			auto rotationEnable = m_cfg.get<bool>(id + ".Ring.Rotation.Enable");
			auto rotationSpeed = m_cfg.get<float>(id + ".Ring.Rotation.Speed");
			if (ImGui::Checkbox("Rotation", &rotationEnable)) {
				if (rotationEnable) {
					m_equalizers[currentEq]->setRotationSpeed(rotationSpeed);
				}
				else {
					m_equalizers[currentEq]->disableRotation();
				}
				m_cfg.put(id + ".Ring.Rotation.Enable", rotationEnable);
			}

			if (rotationEnable) {
				if (ImGui::DragFloat("Rotation speed", &rotationSpeed, .01f * speedMod, -1.f, 1.f)) {
					m_equalizers[currentEq]->setRotationSpeed(rotationSpeed);
					m_cfg.put(id + ".Ring.Rotation.Speed", rotationSpeed);
				}
			}
		}
	}
}

void Editor::renderParticleEffectSettings(const float speedMod) {
	bool enable = m_cfg.get<bool>("Wallpaper.ParticleEffect.Enable");
	if (ImGui::Checkbox("Particle effect enable", &enable)) {
		if (enable) {
			delete m_particleEffect;
			m_particleEffect = new ParticleEffect(m_audioPreProcessor->getSpectrum());
			m_particleEffect->loadSettings(m_cfg.get_child("Wallpaper.ParticleEffect"));
		}
		else {
			delete m_particleEffect;
			m_particleEffect = nullptr;
		}
		m_cfg.put("Wallpaper.ParticleEffect.Enable", enable);
	}

	if (enable) {
		int particleCount = m_cfg.get<int>("Wallpaper.ParticleEffect.ParticleCount");
		if (ImGui::DragInt("Particle count", &particleCount, 1, 1, 500)) {
			m_particleEffect->setParticleCount(particleCount);
			m_cfg.put("Wallpaper.ParticleEffect.ParticleCount", particleCount);
		}

		bool fixedSize = m_cfg.get<bool>("Wallpaper.ParticleEffect.Particle.Size.Fixed");
		if (ImGui::Checkbox("Fixed size", &fixedSize)) {
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Fixed", fixedSize);
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Max", m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Size.Min"));
		}

		if (fixedSize) {
			auto size = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Size.Min");

			if (ImGui::DragFloat("Size", &size, .01f * speedMod, .001f, .5f)) {
				m_particleEffect->setFixedSize(size);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Max", size);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Min", size);
			}
		}
		else {
			auto sizeMin = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Size.Min");
			auto sizeMax = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Size.Max");

			if (ImGui::DragFloat("Size min", &sizeMin, .01f * speedMod, .001f, .5f)) {
				if (sizeMax < sizeMin) {
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Max", sizeMin);
				}
				m_particleEffect->setSize(sizeMin, sizeMax);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Min", sizeMin);
			}

			if (ImGui::DragFloat("Size max", &sizeMax, .01f * speedMod, .001f, .5f)) {
				if (sizeMax < sizeMin) {
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Min", sizeMax);
				}
				m_particleEffect->setSize(sizeMin, sizeMax);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Size.Max", sizeMax);
			}

			if (ImGui::Button("Re-init sizes")) {
				m_particleEffect->reInitSizes();
			}
		}
		ImGui::Text("Velocity");

		bool velLock = m_cfg.get<bool>("Wallpaper.ParticleEffect.Particle.Velocity.Locked");

		if (ImGui::Checkbox("Lock directions", &velLock)) {
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Locked", velLock);
			if (velLock) {
				const auto vel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.X.Max");
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Min", -vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Min", -vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Max", vel);
				m_particleEffect->setVelocity(-vel, vel);
			}
		}

		if (velLock) {
			auto vel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.X.Max");

			if (ImGui::DragFloat("Velocity", &vel, .01f * speedMod, 0.f, .75f)) {
				m_particleEffect->setVelocity(-vel, vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Min", -vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Max", vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Min", -vel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Max", vel);
			}
		}
		else {
			auto velMinX = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.X.Min");
			auto velMaxX = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.X.Max");
			auto velMinY = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Y.Min");
			auto velMaxY = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Y.Max");

			if (ImGui::DragFloat("X min", &velMinX, .01f * speedMod, -.75f, .75f)) {
				if (velMaxX < velMinX) {
					velMaxX = velMinX;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Max", velMinX);
				}
				m_particleEffect->setVelocityX(velMinX, velMaxX);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Min", velMinX);
			}

			if (ImGui::DragFloat("X Max", &velMaxX, .01f * speedMod, -.75f, .75f)) {
				if (velMaxX < velMinX) {
					velMinX = velMaxX;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Min", velMaxX);
				}
				m_particleEffect->setVelocityX(velMinX, velMaxX);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.X.Max", velMaxX);
			}

			if (ImGui::DragFloat("Y min", &velMinY, .01f * speedMod, -.75f, .75f)) {
				if (velMaxY < velMinY) {
					velMaxY = velMinY;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Max", velMinY);
				}
				m_particleEffect->setVelocityY(velMinY, velMaxY);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Min", velMinY);
			}

			if (ImGui::DragFloat("Y max", &velMaxY, .01f * speedMod, -.75f, .75f)) {
				if (velMaxY < velMinY) {
					velMinY = velMaxX;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Min", velMaxY);
				}
				m_particleEffect->setVelocityY(velMinY, velMaxY);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Y.Max", velMaxY);
			}

		}

		if (ImGui::Button("Re-init velocities")) {
			m_particleEffect->reInitVelocities();
		}

		ImGui::Text("Rotation");
		bool angleVelLock = m_cfg.get<bool>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Locked");
		if (ImGui::Checkbox("Lock direction", &angleVelLock)) {
			const auto angleVel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max");
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Locked", angleVelLock);
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", -angleVel);
			m_particleEffect->setAngleVelocity(-angleVel, angleVel);
		}

		if (angleVelLock) {
			auto angleVel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max");
			if (ImGui::DragFloat("Speed", &angleVel, 1.f * speedMod, -360.f, 360.f)) {
				m_particleEffect->setAngleVelocity(-angleVel, angleVel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", -angleVel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max", angleVel);
			}
		}
		else {
			auto angleVelMin = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min");
			auto angleVelMax = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max");
			if (ImGui::DragFloat("Min speed", &angleVelMin, 1.f * speedMod, -360.f, 360.f))
			{
				if (angleVelMax < angleVelMin) {
					angleVelMax = angleVelMin;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max", angleVelMin);
				}
				m_particleEffect->setAngleVelocity(angleVelMin, angleVelMax);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", angleVelMin);
			}

			if (ImGui::DragFloat("Max speed", &angleVelMax, 1.f * speedMod, -360.f, 360.f))
			{
				if (angleVelMax < angleVelMin) {
					angleVelMin = angleVelMax;
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", angleVelMax);
				}
				m_particleEffect->setAngleVelocity(angleVelMin, angleVelMax);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max", angleVelMax);
			}
		}

		if (ImGui::Button("Re-init angular velocity")) {
			m_particleEffect->reInitAngleVelocity();
		}

		if (ImGui::TreeNode("Glow")) {
			bool glowEnable = m_cfg.get<bool>("Wallpaper.ParticleEffect.Particle.Glow.Enable");
			if (ImGui::Checkbox("Enable", &glowEnable)) {
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Enable", glowEnable);
				m_particleEffect->setGlowEnable(glowEnable);
			}

			if (glowEnable) {
				auto glowSizeMin = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Glow.Size.Min");
				auto glowSizeMax = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Glow.Size.Max");

				if (ImGui::DragFloat("Size min", &glowSizeMin, .1f * speedMod, 1.f, 5.f))
				{
					if (glowSizeMax < glowSizeMin) {
						glowSizeMax = glowSizeMin;
						m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Size.Max", glowSizeMin);
					}
					m_particleEffect->setGlowSize(glowSizeMin, glowSizeMax);
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Size.Min", glowSizeMin);
				}

				if (ImGui::DragFloat("Size max", &glowSizeMax, 0.1f * speedMod, 1.f, 5.f))
				{
					if (glowSizeMax < glowSizeMin) {
						glowSizeMin = glowSizeMax;
						m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Size.Min", glowSizeMax);
					}
					m_particleEffect->setGlowSize(glowSizeMin, glowSizeMax);
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Size.Max", glowSizeMax);
				}

				if (ImGui::Button("Re init glow size")) {
					m_particleEffect->reInitGlowSize();
				}

				auto glowAmountMin = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Glow.Amount.Min");
				auto glowAmountMax = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Glow.Amount.Max");

				if (ImGui::DragFloat("Amount min", &glowAmountMin, .01f * speedMod, .01f, 1.f))
				{
					if (glowAmountMax < glowAmountMin) {
						glowAmountMax = glowAmountMin;
						m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Amount.Max", glowAmountMin);
					}
					m_particleEffect->setGlowAmount(glowAmountMin, glowAmountMax);
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Amount.Min", glowAmountMin);
				}

				if (ImGui::DragFloat("Amount max", &glowAmountMax, 0.01f * speedMod, .01f, 1.f))
				{
					if (glowAmountMax < glowAmountMin) {
						glowAmountMin = glowAmountMax;
						m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Amount.Min", glowAmountMax);
					}
					m_particleEffect->setGlowAmount(glowAmountMin, glowAmountMax);
					m_cfg.put("Wallpaper.ParticleEffect.Particle.Glow.Amount.Max", glowAmountMax);
				}

				if (ImGui::Button("Re init glow amount")) {
					m_particleEffect->reInitGlowAmount();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Line")) {
			bool lineEnable = m_cfg.get<bool>("Wallpaper.ParticleEffect.Line.Enable");

			if (ImGui::Checkbox("Enable", &lineEnable)) {
				m_cfg.put("Wallpaper.ParticleEffect.Line.Enable", lineEnable);
				m_particleEffect->setLineEnable(lineEnable);
			}

			auto maxDistance = m_cfg.get<float>("Wallpaper.ParticleEffect.Line.MaxDistance");
			if (ImGui::DragFloat("Max distance", &maxDistance, .01f * speedMod, .001f, 1.f)) {
				m_cfg.put("Wallpaper.ParticleEffect.Line.MaxDistance", maxDistance);
				m_particleEffect->setLineMaxDistance(maxDistance);
			}

			auto lineWidth = m_cfg.get<float>("Wallpaper.ParticleEffect.Line.Width");
			if (ImGui::DragFloat("Width", &lineWidth, .001f * speedMod, .0001f, .05f)) {
				m_cfg.put("Wallpaper.ParticleEffect.Line.Width", lineWidth);
				m_particleEffect->setLineWidth(lineWidth);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Mouse gravity")) {
			auto gravityEnable = m_cfg.get<bool>("Wallpaper.ParticleEffect.MouseGravity.Enable");

			if (ImGui::Checkbox("Enable", &gravityEnable)) {
				m_cfg.put("Wallpaper.ParticleEffect.MouseGravity.Enable", gravityEnable);
				m_particleEffect->setMouseGravityEnable(gravityEnable);
			}

			auto gravityStrength = m_cfg.get<float>("Wallpaper.ParticleEffect.MouseGravity.Strength");
			if (ImGui::DragFloat("Strength", &gravityStrength, 1.f * speedMod, .1f, 30.f)) {
				m_cfg.put("Wallpaper.ParticleEffect.MouseGravity.Strength", gravityStrength);
				m_particleEffect->setMouseGravityStrength(gravityStrength);
			}

			ImGui::TreePop();
		}

		bool audioResponseEnable = m_cfg.get<bool>("Wallpaper.ParticleEffect.AudioResponse.Enable");
		if (ImGui::Checkbox("Particle effect audio response enable", &audioResponseEnable)) {
			m_particleEffect->setAudioResponseEnable(audioResponseEnable);
			m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.Enable", audioResponseEnable);
		}

		if (audioResponseEnable) {
			auto bassPeakAmplifier = m_cfg.get<float>("Wallpaper.ParticleEffect.AudioResponse.BassPeakAmplifier");
			if (ImGui::DragFloat("Bass peak", &bassPeakAmplifier, .1f * speedMod, 0.f, 10.f)) {
				m_particleEffect->setBassPeakAmplifier(bassPeakAmplifier);
				m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.BassPeakAmplifier", bassPeakAmplifier);
			}

			auto highPeakAmplifier = m_cfg.get<float>("Wallpaper.ParticleEffect.AudioResponse.HighPeakAmplifier");
			if (ImGui::DragFloat("High peak", &highPeakAmplifier, .1f * speedMod, 0.f, 10.f)) {
				m_particleEffect->setHighPeakAmplifier(highPeakAmplifier);
				m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.HighPeakAmplifier", highPeakAmplifier);
			}

			if (ImGui::TreeNode("Advanced")) {
				auto bassBaseAmplifier = m_cfg.get<float>("Wallpaper.ParticleEffect.AudioResponse.BassBaseAmplifier");
				if (ImGui::DragFloat("Bass base", &bassBaseAmplifier, .1f * speedMod, .01f, 20.f)) {
					m_particleEffect->setBassBaseAmplifier(bassBaseAmplifier);
					m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.BassBaseAmplifier", bassBaseAmplifier);
				}

				auto highBaseAmplifier = m_cfg.get<float>("Wallpaper.ParticleEffect.AudioResponse.HighBaseAmplifier");
				if (ImGui::DragFloat("High base", &highBaseAmplifier, .1f * speedMod, .01f, 20.f)) {
					m_particleEffect->setHighBaseAmplifier(highBaseAmplifier);
					m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.HighBaseAmplifier", highBaseAmplifier);
				}

				auto colorFilter = m_cfg.get<float>("Wallpaper.ParticleEffect.AudioResponse.ColorFilter");
				if (ImGui::DragFloat("Color filter", &colorFilter, 1.f * speedMod, 0.f, 10.f)) {
					m_particleEffect->setColorFilter(colorFilter);
					m_cfg.put("Wallpaper.ParticleEffect.AudioResponse.ColorFilter", colorFilter);
				}

				ImGui::TreePop();
			}
		}
	}
}

void Editor::loadSettings(const std::string& name) {
	read_xml(name + ".xml", m_cfg);

	if (m_cfg.get<bool>("Wallpaper.Slideshow.Enable")) {
		if (!m_slideShow) {
			m_slideShow = new SlideShow(m_audioPreProcessor->getSpectrum());
		}
		m_slideShow->loadSettings(m_cfg.get_child("Wallpaper.Slideshow"));
	}
	else {
		delete m_slideShow;
		m_slideShow = nullptr;
	}

	if (m_cfg.get<bool>("Wallpaper.ParticleEffect.Enable")) {
		if (!m_particleEffect) {
			m_particleEffect = new ParticleEffect(m_audioPreProcessor->getSpectrum());
		}
		m_particleEffect->loadSettings(m_cfg.get_child("Wallpaper.ParticleEffect"));
	}
	else {
		delete m_particleEffect;
		m_particleEffect = nullptr;
	}
}

void Editor::saveSettings(const std::string& name) const {
	write_xml(name + ".xml", m_cfg);
}
#endif
