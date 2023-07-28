#include "OpenXR.h"
#include <string>
#include "Rendering.h"
#include "Debug.h"
#include <set>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#define XR_USE_PLATFORM_WIN64 // Use the appropriate platform, e.g., XR_USE_PLATFORM_XLIB for Linux
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>


namespace OpenXR {

    XrInstance openxrInstance;
    XrSystemId systemId;
    XrSession session;
    uint32_t viewCount;
    XrSwapchain xrSwapchain;
    XrSwapchainImageBaseHeader* swapchainImageData;
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;
    uint32_t swapchainWidth;
    uint32_t swapchainHeight;
    std::vector<const char*> enabledExtensions;
    static std::vector<char> extensionStringBuffer;
    static std::vector<const char*> extensions;
    XrSpace localSpace;
    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;


    std::string initOpenXR() {

        const char* gameTitle = UnseenEngine::GAME_TITLE.c_str();
        const char* engineTitle = UnseenEngine::ENGINE_TITLE.c_str();

        
        enabledExtensions.push_back("XR_KHR_vulkan_enable");
        
        //Create instance
        XrInstanceCreateInfo instanceCreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
        instanceCreateInfo.enabledApiLayerCount = 0;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        instanceCreateInfo.enabledExtensionNames = enabledExtensions.data();
        instanceCreateInfo.applicationInfo = { *gameTitle, 1, *engineTitle, 1 };
        instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

        XrResult result = xrCreateInstance(&instanceCreateInfo, &openxrInstance);
        if (result != XR_SUCCESS) {
            return "Failed to create OpenXR instance.";
        }

        //Get system
        XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO };
        systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        XrResult systemResult = xrGetSystem(openxrInstance, &systemGetInfo, &systemId);
        if (systemResult != XR_SUCCESS) {
            return "Oculus Quest 2 not found - make sure a Quest is connected to the pc via AirLink.";
        }

        return "Success";

    }

    void runOpenXR(){

        createSession();
        createVRSwapchain();
        createLocalSpace();
        beginSession();


    }

    void createSession() {

        //Get system
        XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO };
        systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        XrResult systemResult = xrGetSystem(openxrInstance, &systemGetInfo, &systemId);
        if (systemResult != XR_SUCCESS) {
            debug.log("Oculus Quest 2 not found - make sure a Quest is connected to the pc via AirLink.");
        }

        // Check to see if we can find a compatible physical device
        PFN_xrGetVulkanGraphicsDeviceKHR pfnGetVulkanGraphicsDeviceKHR;
        xrGetInstanceProcAddr(openxrInstance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanGraphicsDeviceKHR));
        VkPhysicalDevice compatiblePhysicalDevice;
        XrResult compatibleDeviceResult = pfnGetVulkanGraphicsDeviceKHR(openxrInstance, systemId, Rendering::instance, &compatiblePhysicalDevice);
        if (compatibleDeviceResult != XR_SUCCESS) {
            debug.log("Failed to find a compatible Vulkan physical device for OpenXR");
        }

        //obtaining the graphics requirements

        PFN_xrGetVulkanInstanceExtensionsKHR pfnGetVulkanInstanceExtensionsKHR;
        PFN_xrGetVulkanDeviceExtensionsKHR pfnGetVulkanDeviceExtensionsKHR;
        PFN_xrGetVulkanGraphicsRequirementsKHR pfnGetVulkanGraphicsRequirementsKHR;
        xrGetInstanceProcAddr(openxrInstance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanInstanceExtensionsKHR));
        xrGetInstanceProcAddr(openxrInstance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanDeviceExtensionsKHR));
        xrGetInstanceProcAddr(openxrInstance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanGraphicsRequirementsKHR));
        uint32_t instanceExtensionCount;
        pfnGetVulkanInstanceExtensionsKHR(openxrInstance, systemId, 0, &instanceExtensionCount, nullptr);
        std::vector<char> instanceExtensionsBuffer(instanceExtensionCount);
        pfnGetVulkanInstanceExtensionsKHR(openxrInstance, systemId, instanceExtensionCount, &instanceExtensionCount, instanceExtensionsBuffer.data());
        uint32_t deviceExtensionCount;
        pfnGetVulkanDeviceExtensionsKHR(openxrInstance, systemId, 0, &deviceExtensionCount, nullptr);
        std::vector<char> deviceExtensionsBuffer(deviceExtensionCount);
        pfnGetVulkanDeviceExtensionsKHR(openxrInstance, systemId, deviceExtensionCount, &deviceExtensionCount, deviceExtensionsBuffer.data());
        XrGraphicsRequirementsVulkanKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
        pfnGetVulkanGraphicsRequirementsKHR(openxrInstance, systemId, &graphicsRequirements);


        //Create the session
        Rendering::QueueFamilyIndices indices = Rendering::findQueueFamilies(Rendering::physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        XrGraphicsBindingVulkanKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
        graphicsBinding.instance = Rendering::instance;
        graphicsBinding.physicalDevice = Rendering::physicalDevice;
        graphicsBinding.queueFamilyIndex = indices.graphicsFamily.value();
        graphicsBinding.queueIndex = 0;
        graphicsBinding.device = Rendering::device;

        XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
        sessionCreateInfo.next = &graphicsBinding;
        sessionCreateInfo.systemId = systemId;

        XrResult result = xrCreateSession(openxrInstance, &sessionCreateInfo, &session);
        if (result != XR_SUCCESS) {
            debug.log("Failed to create OpenXR session.");
        }

    }


    void createVRSwapchain() {

        //Query OpenXR view configuration

        xrEnumerateViewConfigurationViews(openxrInstance, systemId, viewConfigurationType, 0, &viewCount, nullptr);
        std::vector<XrViewConfigurationView> viewConfigurationViews(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
        xrEnumerateViewConfigurationViews(openxrInstance, systemId, viewConfigurationType, viewCount, &viewCount, viewConfigurationViews.data());

        //Create the OpenXR swapchain
        uint32_t swapchainFormatCount;
        xrEnumerateSwapchainFormats(session, 0, &swapchainFormatCount, nullptr);
        std::vector<int64_t> swapchainFormats(swapchainFormatCount);
        xrEnumerateSwapchainFormats(session, swapchainFormatCount, &swapchainFormatCount, swapchainFormats.data());

        // Choose a swapchain format that is compatible with your Vulkan device
        int64_t chosenSwapchainFormat = chooseSwapchainFormat(swapchainFormats, Rendering::physicalDevice, Rendering::surface);


        XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.format = chosenSwapchainFormat;
        swapchainCreateInfo.sampleCount = viewConfigurationViews[0].recommendedSwapchainSampleCount;
        swapchainCreateInfo.width = viewConfigurationViews[0].recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfigurationViews[0].recommendedImageRectHeight;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.arraySize = viewCount;
        swapchainCreateInfo.mipCount = 1;

        swapchainWidth = swapchainCreateInfo.width;
        swapchainHeight = swapchainCreateInfo.height;


        XrResult result = xrCreateSwapchain(session, &swapchainCreateInfo, &xrSwapchain);
        if (result != XR_SUCCESS) {
            debug.log("Failed to create OpenXR swapchain.");
        }

        uint32_t swapchainImageCount;
        xrEnumerateSwapchainImages(xrSwapchain, 0, &swapchainImageCount, nullptr);
        std::vector<XrSwapchainImageVulkanKHR> swapchainImages(swapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
        xrEnumerateSwapchainImages(xrSwapchain, swapchainImageCount, &swapchainImageCount, reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()));
        swapchainImageData = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data());

    }

    void createLocalSpace() {

        // Create a local space

        XrReferenceSpaceCreateInfo localSpaceCreateInfo = {};
        localSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
        localSpaceCreateInfo.next = nullptr;
        localSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        localSpaceCreateInfo.poseInReferenceSpace.orientation.x = 0.0f;
        localSpaceCreateInfo.poseInReferenceSpace.orientation.y = 0.0f;
        localSpaceCreateInfo.poseInReferenceSpace.orientation.z = 0.0f;
        localSpaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
        localSpaceCreateInfo.poseInReferenceSpace.position.x = 0.0f;
        localSpaceCreateInfo.poseInReferenceSpace.position.y = 0.0f;
        localSpaceCreateInfo.poseInReferenceSpace.position.z = 0.0f;

        XrResult result = xrCreateReferenceSpace(session, &localSpaceCreateInfo, &localSpace);
        if (XR_FAILED(result)) {
            // Handle error
        }

    }

    int64_t chooseSwapchainFormat(const std::vector<int64_t>& supportedSwapchainFormats, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

        // Iterate through the available surface formats and compare to the supported OpenXR swapchain formats
        for (const auto& surfaceFormat : surfaceFormats) {
            for (const auto& swapchainFormat : supportedSwapchainFormats) {
                if (surfaceFormat.format == swapchainFormat) {
                    return swapchainFormat;
                }
            }
        }

        // If no compatible format is found, return the first supported OpenXR format
        return supportedSwapchainFormats[0];
    }

    std::vector<const char*> splitExtensions(const std::string& extensionsStr) {
        std::vector < const char*> extensions;
        std::size_t start = 0;
        std::size_t end = 0;

        while ((end = extensionsStr.find(' ', start)) != std::string::npos) {
            std::string ext = extensionsStr.substr(start, end - start);
            extensions.push_back(_strdup(ext.c_str()));
            start = end + 1;
        }

        extensions.push_back(_strdup(extensionsStr.substr(start).c_str()));
        return extensions;
    }

    void beginSession() {

        XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        sessionBeginInfo.primaryViewConfigurationType = OpenXR::viewConfigurationType;

        //Begin session
        XrResult result = xrBeginSession(OpenXR::session, &sessionBeginInfo);
        if (XR_FAILED(result)) {
            debug.log("Failed to begin OpenXR session!");
        }

    }


}