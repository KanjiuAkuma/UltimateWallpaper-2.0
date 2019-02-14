#pragma once

#include <boost/property_tree/ptree.hpp>

class Configurable {
public:
	explicit Configurable() = default;
	virtual ~Configurable() = default;

	virtual void loadSettings(boost::property_tree::ptree& configuration) = 0;
};