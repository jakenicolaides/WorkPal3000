#pragma once
#include <string>
#include "vulkan/vulkan.h"
#include <vector>
#define XR_USE_PLATFORM_WIN64 
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>


namespace OpenXR {

    extern uint32_t viewCount;
    extern XrSwapchain xrSwapchain;
    extern XrSession session;
    extern XrSwapchainImageBaseHeader* swapchainImageData;
    extern XrSpace localSpace;
    extern XrViewConfigurationType viewConfigurationType;
    extern uint32_t swapchainHeight;
    extern uint32_t swapchainWidth;


    std::string initOpenXR();
    void beginSession();
    void runOpenXR();
    void createSession();
    int64_t chooseSwapchainFormat(const std::vector<int64_t>& supportedSwapchainFormats, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    std::vector<const char*> splitExtensions(const std::string& extensionsStr);
    //const char** getOpenXRRequiredExtensions(uint32_t* openXRExtensionCount);
    void createVRSwapchain();
   // void updateRenderingPipeline(XrSession& session);

    void createLocalSpace();

}
