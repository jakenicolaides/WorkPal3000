/*
#pragma once
#include "Rendering.h"
#include "vk_mem_alloc.h"
#include <gtc/quaternion.hpp>
#include <string>
#include <random>


namespace Data {

    static const int numEntities = 100;

    struct Entities {
        int id[numEntities] = {0};
        glm::mat4 transform[numEntities] = { glm::mat4(1.0f) };
        glm::quat rotation[numEntities] = { glm::quat(1.0f, 0.0f, 0.0f, 0.0f) };
        std::string typePath[numEntities] = {""};
        int renderedModelId[numEntities] = {0};
    };

    struct RenderedModels {
        int id[numEntities] = { 0 };
        std::vector<uint32_t> indices[numEntities];
        VkBuffer vertexBuffer[numEntities];
        VkBuffer indexBuffer[numEntities];
        uint32_t index[numEntities];
        VkDeviceMemory indexBufferMemory[numEntities];
        VkDeviceMemory vertexBufferMemory[numEntities];
        VmaAllocation vertexAllocation[numEntities];
        VmaAllocation indexAllocation[numEntities];
        std::vector<VkDescriptorSet> descriptorSets[numEntities];
    };

    void newEntity(std::string typePath);
    int initialiseNewEntry(int id[], int size);

    int getEntityIndexFromId(int entityId);

    extern Entities entities;
    extern RenderedModels renderedModels;

  

}

*/