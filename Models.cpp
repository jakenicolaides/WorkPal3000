#include "Models.h"
#include "Debug.h"
#include <set>

namespace Models {

    void assimpLoadModel(const std::string& modelPath, std::vector<Rendering::Vertex>& modelVertices, std::vector<uint32_t>& modelIndices, std::string& textureFileName) {
        Assimp::Importer importer;
        std::string filePath = modelPath;
        const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene) { debug.log("Error: Failed to load model file " + filePath + ". " + importer.GetErrorString()); return; }
        assimpProcessNode(scene->mRootNode, scene, modelVertices, modelIndices, textureFileName);
    }

    void assimpProcessNode(aiNode* node, const aiScene* scene, std::vector<Rendering::Vertex>& verts, std::vector<uint32_t>& indices, std::string& textureFileName) {
        // Process each mesh in the node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            assimpProcessMesh(mesh, scene, verts, indices, textureFileName);
        }

        // Process the node's children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            assimpProcessNode(node->mChildren[i], scene, verts, indices, textureFileName);
        }
    }

    void assimpProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Rendering::Vertex>& verts, std::vector<uint32_t>& indices, std::string& textureFileName) {

        // Process the vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Rendering::Vertex vertex;
            // Extract the position, normal, and texture coordinates of the vertex
            vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            if (mesh->mTextureCoords[0]) { // Does the mesh contain texture coordinates?
                vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            else {
                vertex.texCoord = glm::vec2(0.0f, 0.0f);
            }
            verts.push_back(vertex);
        }


        // Process the indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        


        // Iterate through all materials in the scene

        std::vector<std::string> texturePaths;
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* material = scene->mMaterials[i];

            // Iterate through all texture types (diffuse, specular, etc.)
            aiTextureType textureTypes[] = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS };
            for (unsigned int j = 0; j < sizeof(textureTypes) / sizeof(textureTypes[0]); j++) {
                aiTextureType textureType = textureTypes[j];

                // Iterate through all textures of the current type
                for (unsigned int k = 0; k < material->GetTextureCount(textureType); k++) {
                    aiString texturePath;

                    // Get the texture file path
                    if (material->GetTexture(textureType, k, &texturePath) == AI_SUCCESS) {
                        texturePaths.push_back(texturePath.C_Str());
                    }
                }
            }
        }

        if (texturePaths.size() > 0) {
            textureFileName = texturePaths[0];
        }
       

    }

    

}
