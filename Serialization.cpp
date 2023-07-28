#include "Serialization.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "Debug.h"
#include "Functions.h"
#include <windows.h>


namespace Serialization {
	
    void generateYamlFile(const std::string& path, const std::string& filename, const int model_id) {

        // Combine the path and filename to form the full file path
        std::string fullpath = path + "/" + filename + ".entity";

        // Create a YAML::Emitter object to generate YAML output
        YAML::Emitter emitter;

        // Start the YAML document
        emitter << YAML::BeginMap;

        // Add the model_id property to the document
        emitter << YAML::Key << "model_id";
        emitter << YAML::Value << model_id;

        // End the YAML document
        emitter << YAML::EndMap;

        // Write the YAML output to a file
        std::ofstream fout(fullpath);
        fout << emitter.c_str();
        fout.close();

        // Print a message to confirm that the file was generated
        std::cout << "Generated YAML file: " << fullpath << "\n";
    }

    void newType(const std::string& path, const std::string& typeName) {

        // Combine the path and filename to form the full file path
        std::string fullpath = path + "/" + typeName + ".type";

        // Create a YAML::Emitter object to generate YAML output
        YAML::Emitter emitter;

        // Start the YAML document
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "name";
        emitter << YAML::Value << typeName;
        emitter << YAML::Key << "model_path";
        emitter << YAML::Value << "";

        // End the YAML document
        emitter << YAML::EndMap;

        // Write the YAML output to a file
        std::ofstream fout(fullpath);
        fout << emitter.c_str();
        fout.close();

        // Print a message to confirm that the file was generated
        debug.log("Created new type: " + fullpath);
    }

    std::map<std::string, std::string> yamlFileAsStringMap(const std::string& filePath) {
        // Load the YAML file
        YAML::Node config = YAML::LoadFile(filePath);

        // Convert the YAML file to a map of strings
        std::map<std::string, std::string> yaml_data;
        for (const auto& entry : config) {
            std::string key = entry.first.as<std::string>();
            std::string value = entry.second.as<std::string>();
            yaml_data[key] = value;
        }
        return yaml_data;
    }

    void updateType(const std::string& typePath, const std::string& key, const std::string& new_value) {

        /// Load the YAML file
        YAML::Node typeFile = YAML::LoadFile(typePath);

        // Check if the key exists in the YAML file
        if (!typeFile[key]) {
            debug.log("Error: type key not found");
            return;
        }

        // Update the value of the key
        typeFile[key] = new_value;

        // Set the file attribute to not read-only
        if (!SetFileAttributes(Functions::ConvertToLPWSTR(typePath), FILE_ATTRIBUTE_NORMAL)) {
            debug.log("Error: failed to set file attribute to normal");
            return;
        }

        // Write the updated YAML file back to disk
        std::ofstream fout(typePath);
        fout << typeFile;

    }


	

}
