#pragma once
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Serialization {



	void generateYamlFile(const std::string& path, const std::string& filename, const int model_id);

	void newType(const std::string& path, const std::string& typeName);

	std::map<std::string, std::string> yamlFileAsStringMap(const std::string& filePath);

	void updateType(const std::string& typePath, const std::string& key, const std::string& new_value);

}

