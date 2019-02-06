#pragma once

#include <application/Application.h>

using namespace JApp;

class UltimateWallpaper : public Application {

public:
	void render() override;
	void update(float dt) override;
};
