#pragma once
#include "Rendering.h"
#include <vector>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

namespace Models {

    extern Assimp::Importer importer;
    void assimpLoadModel(const std::string& modelPath, std::vector<Rendering::Vertex>& modelVertices, std::vector<uint32_t>& modelIndices, std::string& fileName);
    void assimpProcessNode(aiNode* node, const aiScene* scene, std::vector<Rendering::Vertex>& verts, std::vector<uint32_t>& indices, std::string& fileName);
    void assimpProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Rendering::Vertex>& verts, std::vector<uint32_t>& indices, std::string& fileName);
}