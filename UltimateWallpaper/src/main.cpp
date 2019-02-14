#include <entryPoint.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#if SHOW_EDITOR
#include "editor/Editor.h"
#else
#include "UltimateWallpaper.h"
#endif

using namespace JApp;

Application* JApp::createApplication() {
	// Create empty property tree object
	boost::property_tree::ptree tree;

	// Parse the XML into the property tree.
	read_xml("cfg.xml", tree);

	// create application
	#if SHOW_EDITOR
	return new Editor(tree);
	#else
	return new UltimateWallpaper(tree);
	#endif
}