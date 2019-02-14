#include "SlideShow.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <logger/Log.h>
#include <macros.h>


#include "transition/AlphaTransition.h"
#include "glm/gtx/integer.inl"

SlideShow::SlideShow(Spectrum* spectrum)
{
	m_audioSpectrum = spectrum;
}

SlideShow::~SlideShow() {
	delete m_transition;
}

void SlideShow::update(const float dt) {
	if (m_swapEnable) {

		m_dtAccumulated += dt;
		m_transition->update(dt);


		if (m_updateInterval < m_dtAccumulated) {
			nextImage();
			preLoadImage(randomImage());
		}
	}
}

void SlideShow::render(const glm::mat4 projection) const {
	// sub bass + bass: 60 -> 250 Hz
	float bassAmplitude = 0;
	float highAmplitude = 0;
	if (m_audioResponseEnable) {
		bassAmplitude = m_audioSpectrum->sumRange(LOW_END, BASS, 5.f);
		bassAmplitude = log(1.f + bassAmplitude * m_bassBaseAmplifier) * m_bassPeakAmplifier;

		highAmplitude = m_audioSpectrum->sumRange(MID_RANGE, PRESENCE, 1.1f);
		highAmplitude = log(1.f + highAmplitude * m_highBaseAmplifier) * m_highPeakAmplifier;
	}

	m_transition->render(projection, bassAmplitude, highAmplitude);
}

void SlideShow::loadSettings(boost::property_tree::ptree& configuration) {
	m_imageDuration = configuration.get<int>("ImageDuration");
	const std::string tType = configuration.get<std::string>("Transition.Type");
	delete m_transition;
	if (tType == "Alpha") {
		m_transition = new AlphaTransition(configuration.get<float>("Transition.Duration"));
	}
	else {
		m_transition = new AlphaTransition(configuration.get<float>("Transition.Duration"));
	}
	m_updateInterval = m_imageDuration + m_transition->getDuration();

	m_audioResponseEnable = configuration.get<bool>("AudioResponse.Enable");
	m_bassBaseAmplifier = configuration.get<float>("AudioResponse.BassBaseAmplifier");
	m_bassPeakAmplifier = configuration.get<float>("AudioResponse.BassPeakAmplifier");
	m_highBaseAmplifier = configuration.get<float>("AudioResponse.HighBaseAmplifier");
	m_highPeakAmplifier = configuration.get<float>("AudioResponse.HighPeakAmplifier");
	// load images
	setImageDirectory(configuration.get<std::string>("ImageDirectory"));
}

void SlideShow::setImageDirectory(const std::string& imageDirectory) {
	m_imageDirectory = imageDirectory;
	m_filePaths.clear();
	scanDirectory(imageDirectory);

	if (!imageDirectory.empty()) {
		APP_INFO("Found {:d} images", m_filePaths.size());

		if (m_filePaths.size() < 2) {
			APP_WARN("Disable swap, not enough images");
			m_swapEnable = false;
			if (m_filePaths.size() == 1) {
				APP_TRACE("Loading only image");
				preLoadFirstImage(m_filePaths[0]);
				firstImage();
			}
		}
		else {
			if (m_imageDuration < 1) {
				APP_TRACE("Disable swap, duration < 1");
				m_swapEnable = false;
				// load random image from folder
				if (m_imageDuration == 0) {
					APP_TRACE("Loading single image");
					preLoadFirstImage(randomImage());
					firstImage();
				}
			}
			else {
				m_swapEnable = true;
				// load first image and pre load second
				preLoadFirstImage(randomImage());
				firstImage();
				preLoadImage(randomImage());
				RELEASE
				(
					nextImage();
				preLoadImage(randomImage());
				)
			}
		}
	}
	else {
		m_swapEnable = false;
	}
}

#if SHOW_EDITOR
void SlideShow::setImageDuration(const int duration) {
	m_imageDuration = duration;
	m_updateInterval = duration + m_transition->getDuration();
}

void SlideShow::setTransition(Transition* transition) {
	delete m_transition;
	m_transition = transition;
}

void SlideShow::setTransitionDuration(const float transitionDuration) const {
	m_transition->setDuration(transitionDuration);
}

void SlideShow::setAudioResponseEnable(const bool audioResponseEnable) {
	m_audioResponseEnable = audioResponseEnable;
}

void SlideShow::setBassPeakAmplifier(const float peakAmplifier) {
	m_bassPeakAmplifier = peakAmplifier;
}

void SlideShow::setHighPeakAmplifier(const float peakAmplifier) {
	m_highPeakAmplifier = peakAmplifier;
}

void SlideShow::setBassBaseAmplifier(const float baseAmplifier) {
	m_bassBaseAmplifier = baseAmplifier;
}

void SlideShow::setHighBaseAmplifier(const float baseAmplifier) {
	m_highBaseAmplifier = baseAmplifier;
}

#endif

void SlideShow::scanDirectory(const std::string& directory) {
	APP_INFO("Searching images in '{}'", directory);

	std::string searchPath = directory + "/*.*";

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				m_filePaths.push_back(directory + "/" + fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
}

void SlideShow::preLoadFirstImage(const std::string& path) {
	m_imageLoader = new JApp::ImageLoader(path);
	m_imageLoader->start();
}

void SlideShow::preLoadImage(const std::string& path) {
	if (!m_imageLoader->isFinished()) {	// push loading image
		nextImage();
	}

	delete m_imageLoader;
	m_imageLoader = new JApp::ImageLoader(path);
	m_imageLoader->start();
}

void SlideShow::firstImage() {
	if (!m_imageLoader->isFinished()) {	// wait for texture
		m_imageLoader->join();
	}

	m_dtAccumulated = 0;
	m_transition->goToEnd();

	// load next image
	m_nextTexture = JApp::Renderer::Texture::fromValues(m_imageLoader->getImage(), m_imageLoader->getImageWidth(), m_imageLoader->getImageHeight());
	m_nextTexture->bind();
	m_transition->setTex1Slot(m_nextTexture->getSlot());
}

void SlideShow::nextImage() {
	if (!m_imageLoader->isFinished()) {	// wait for texture
		APP_WARN("SlideShow: Waiting for image to finish loading!");
		m_imageLoader->join();
	}

	m_dtAccumulated = 0;
	m_transition->goToStart();

	// load next image

	delete m_currentTexture;
	m_currentTexture = m_nextTexture;
	m_nextTexture = JApp::Renderer::Texture::fromValues(m_imageLoader->getImage(), m_imageLoader->getImageWidth(), m_imageLoader->getImageHeight());

	m_currentTexture->bind();
	m_transition->setTex0Slot(m_currentTexture->getSlot());
	m_nextTexture->bind();
	m_transition->setTex1Slot(m_nextTexture->getSlot());
}

std::string SlideShow::randomImage() {
	return m_filePaths[m_filePaths.size() * rand() / RAND_MAX];
}
