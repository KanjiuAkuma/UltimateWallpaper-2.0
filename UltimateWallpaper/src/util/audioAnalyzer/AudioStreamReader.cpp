#include "AudioStreamReader.h"

#include <bass.h>
#include <basswasapi.h>

#include <logger/Log.h>
#include <macros.h>

// Bass callback
DWORD CALLBACK callback(void *buffer, DWORD length, void *user) {
	return true; // continue
}

// error checking
void error(const char *es) {
	// printf("%s\n(error code: %d)", es, BASS_ErrorGetCode());
	APP_ERROR("{} Error code: {}", es, BASS_ErrorGetCode());
}

AudioStreamReader::AudioStreamReader() {
	int devnum = -1;
	BASS_WASAPI_DEVICEINFO info;

	for (int a = 0; BASS_WASAPI_GetDeviceInfo(a, &info); a++) {
		const bool isOutput = !(info.flags & BASS_DEVICE_INPUT);
		const bool isDefault = info.flags & BASS_DEVICE_DEFAULT;
		// printf("Device at %d: sampleRate=%lu, channels=%lu, name='%s'\n", a, info.mixfreq, info.mixchans, info.name);
		APP_TRACE("Device at {}: sampleRate={}, channels={}, name='{}'", a, info.mixfreq, info.mixchans, info.name);

		if (isOutput && isDefault) { // and it is the default
			devnum = a + 1; // use it (+1 because the next device is the corresponding loopback device)
			break;
		}
	}

	if (devnum >= 0) {
		APP_INFO("Using device {}", devnum);

		// initialize bass with no sound
		if (!BASS_Init(0, 48000, 0, nullptr, nullptr)) {
			error("Can not initialize bass!");
		}

		// initialize the device with buffering enabled
		if (!BASS_WASAPI_Init(devnum, info.mixfreq, info.mixchans, BASS_WASAPI_BUFFER, 1, 0, &callback, nullptr)) {
			error("Can not initialize bass wasapi!");
		}
	}
	else {
		error("Can not find a device");
	}

	m_spectrum = new Spectrum(m_fftSize / 2, int(info.mixfreq));
	BASS_WASAPI_Start();
}

AudioStreamReader::~AudioStreamReader() {
	delete m_spectrum;
}

void AudioStreamReader::updateSpectrum() const {
	const int read = BASS_WASAPI_GetData(m_spectrum->data, m_bassFftSize);
	if (read < 0) {
		error("Could not read data!");
	}

	for (int i = 1; i < m_spectrum->size; i++) {
		m_spectrum->data[i] = sqrt(m_spectrum->data[i]);
	}
}

Spectrum* AudioStreamReader::getSpectrum() const {
	return m_spectrum;
}

AudioStreamReader* AudioStreamReader::getInstance() {
	static AudioStreamReader* INSTANCE = new AudioStreamReader();
	return INSTANCE;
}
