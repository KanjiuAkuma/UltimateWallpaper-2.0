#pragma once
#if SHOW_EDITOR

#include <boost/property_tree/ptree.hpp>

#include "UltimateWallpaper.h"

class Editor : public UltimateWallpaper {
public:
	explicit Editor(boost::property_tree::ptree& configuration);
	~Editor();

	void render() override;

	void loadSettings(const std::string& name);
	void saveSettings(const std::string& name) const;

private:
	boost::property_tree::ptree m_cfg;
	char* m_currentName = "cfg";
	bool m_fpsCounterEnableOverride;
};

#endif