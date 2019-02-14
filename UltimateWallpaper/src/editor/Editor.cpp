#if SHOW_EDITOR

#include "Editor.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <boost/property_tree/xml_parser.hpp>

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

	if (ImGui::CollapsingHeader("Slideshow")) {
		bool slideShowEnable = bool(m_slideShow);
		if (ImGui::Checkbox("Enable", &slideShowEnable)) {
			if (slideShowEnable) {
				m_slideShow = new SlideShow(m_audioPreProcessor->getSpectrum());
				m_slideShow->loadSettings(m_cfg.get_child("Wallpaper.Slideshow"));
			}
			else {
				delete m_slideShow;
				m_slideShow = nullptr;	
			}
			m_cfg.put("Wallpaper.Slideshow.Enable", slideShowEnable);
		}

		if (slideShowEnable) {
			ImGui::PushItemWidth(-200);
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

			ImGui::PopItemWidth();

			if (ImGui::TreeNode("Transition")) {
				static char* transitions[]{ "Alpha", "Not Alpha" };
				int selection = 0;
				std::string tranType = m_cfg.get<std::string>("Wallpaper.Slideshow.Transition.Type");
				if (tranType != "Alpha") {
					selection = 1;
				}

				if (ImGui::Combo("Type", &selection, transitions, 2)) {
					APP_INFO("Changed transition type to {}", transitions[selection]);
				}

				float tDur = m_cfg.get<float>("Wallpaper.Slideshow.Transition.Duration");
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
			if (ImGui::Checkbox("Audio response", &audioResponseEnable)) {
				m_slideShow->setAudioResponseEnable(audioResponseEnable);
				m_cfg.put("Wallpaper.Slideshow.AudioResponse.Enable", audioResponseEnable);
			}

			if (audioResponseEnable) {
				auto bassPeakAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.BassPeakAmplifier");
				ImGui::PushItemWidth(-75);
				if (ImGui::DragFloat("Bass peak", &bassPeakAmplifier, .1f * speedMod, .01f, 10.f)) {
					m_slideShow->setBassPeakAmplifier(bassPeakAmplifier);
					m_cfg.put("Wallpaper.Slideshow.AudioResponse.BassPeakAmplifier", bassPeakAmplifier);
				}

				auto highPeakAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.HighPeakAmplifier");
				if (ImGui::DragFloat("High peak", &highPeakAmplifier, .1f * speedMod, .01f, 10.f)) {
					m_slideShow->setHighPeakAmplifier(highPeakAmplifier);
					m_cfg.put("Wallpaper.Slideshow.AudioResponse.HighPeakAmplifier", highPeakAmplifier);
				}
				ImGui::PopItemWidth();

				if (ImGui::TreeNode("Advanced")) {
					ImGui::PushItemWidth(-75);
					auto bassBaseAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.BassBaseAmplifier");
					if (ImGui::DragFloat("Bass base", &bassBaseAmplifier, .1f * speedMod, .01f, 10.f)) {
						m_slideShow->setBassBaseAmplifier(bassBaseAmplifier);
						m_cfg.put("Wallpaper.Slideshow.AudioResponse.BassBaseAmplifier", bassBaseAmplifier);
					}

					auto highBaseAmplifier = m_cfg.get<float>("Wallpaper.Slideshow.AudioResponse.HighBaseAmplifier");
					if (ImGui::DragFloat("High base", &highBaseAmplifier, .1f * speedMod, .01f, 10.f)) {
						m_slideShow->setHighBaseAmplifier(highBaseAmplifier);
						m_cfg.put("Wallpaper.Slideshow.AudioResponse.HighBaseAmplifier", highBaseAmplifier);
					}
					ImGui::PopItemWidth();

					ImGui::TreePop();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Particle Effect")) {
		bool particleEffectEnable = bool(m_particleEffect);
		if (ImGui::Checkbox("Enable", &particleEffectEnable)) {
			if (particleEffectEnable) {
				m_particleEffect = new ParticleEffect();
				m_particleEffect->loadSettings(m_cfg.get_child("Wallpaper.ParticleEffect"));
			}
			else {
				delete m_particleEffect;
				m_particleEffect = nullptr;
			}
		}

		ImGui::PushItemWidth(-110);
		int particleCount = m_cfg.get<int>("Wallpaper.ParticleEffect.ParticleCount");
		if (ImGui::DragInt("Particle count", &particleCount, 1, 1, 500)) {
			m_particleEffect->setParticleCount(particleCount);
			m_cfg.put("Wallpaper.ParticleEffect.ParticleCount", particleCount);
		}
		ImGui::PopItemWidth();

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

			ImGui::PushItemWidth(-80);
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
			ImGui::PopItemWidth();

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

			ImGui::PushItemWidth(-90);
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

			ImGui::PopItemWidth();
		}

		if (ImGui::Button("Re-init velocities")) {
			m_particleEffect->reInitVelocities();
		}

		ImGui::Text("Rotation");
		bool angleVelLock = m_cfg.get<bool>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Locked");
		if (ImGui::Checkbox("Lock direction", &angleVelLock)) {
			const auto angleVel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max");
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Locked", angleVelLock);
			m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", angleVel);
			m_particleEffect->setAngleVelocity(-angleVel, angleVel);
		}

		if (angleVelLock) {
			auto angleVel = m_cfg.get<float>("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Max");
			if (ImGui::DragFloat("Speed", &angleVel, 1.f * speedMod, -360.f, 360.f)) {
				m_particleEffect->setVelocityY(-angleVel, angleVel);
				m_cfg.put("Wallpaper.ParticleEffect.Particle.Velocity.Angle.Min", angleVel);
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
	}

	wpos = ImGui::GetWindowPos();
	ImVec2 wsize = ImGui::GetWindowSize();

	if (!ImGui::IsMouseDown(0)) {
		if (wpos.x - 20.f < 0.f) {
			wpos.x = 0.f;
			clip = true;
		}
		else if (m_windowWidth - wsize.x - 20.f < wpos.x) {
			wpos.x = m_windowWidth - wsize.x;
			clip = true;
		}

		if (wpos.y - 20.f < 0.f) {
			wpos.y = 0.f;
			clip = true;
		}
		else if (m_windowHeight - wsize.y - 20.f < wpos.y) {
			wpos.y = m_windowHeight - wsize.y;
			clip = true;
		}
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
			m_particleEffect = new ParticleEffect();
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
