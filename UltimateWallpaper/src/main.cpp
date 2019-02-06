#include <entryPoint.h>

#include "UltimateWallpaper.h"

using namespace JApp;

Application* JApp::createApplication() {
	return new UltimateWallpaper();
}