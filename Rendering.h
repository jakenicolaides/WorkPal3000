#pragma once
#define GLFW_INCLUDE_VULKAN
#include <fstream>
#include <glfw3.h>
#include <optional>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <string>
#include <vector>
#include <map>


namespace Rendering {

    

    //Consts
    const uint32_t WIDTH = 650;
    const uint32_t HEIGHT = 636;
    
    const int MAX_FRAMES_IN_FLIGHT = 2;

    #ifdef NDEBUG
    const bool enableValidationLayers = false;
    #else
    const bool enableValidationLayers = true;
    #endif

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = {  VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MULTIVIEW_EXTENSION_NAME, "VK_KHR_external_memory", "VK_KHR_external_memory_win32", "VK_KHR_external_fence", "VK_KHR_external_fence_win32", "VK_KHR_external_semaphore", "VK_KHR_external_semaphore_win32", "VK_KHR_get_memory_requirements2", "VK_KHR_dedicated_allocation"};

    //Structs
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete();
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct Triangle {
        uint32_t indexes[3] = { 0, 0, 0 };
        glm::vec3 vertices[3] = { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) };
    };

    // A struct to manage data related to one image in vulkan
    struct ImGuiTextureData
    {
        VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
        int             Width;
        int             Height;
        int             Channels;

        // Need to keep track of these to properly cleanup
        VkImageView     ImageView;
        VkImage         Image;
        VkDeviceMemory  ImageMemory;
        VkSampler       Sampler;
        VkBuffer        UploadBuffer;
        VkDeviceMemory  UploadBufferMemory;

        ImGuiTextureData() { memset(this, 0, sizeof(*this)); }
    };

    //Globals
    //Vukan 
    extern GLFWwindow* window;
    extern VkInstance instance;
    extern VkDebugUtilsMessengerEXT debugMessenger;
    extern VkSurfaceKHR surface;
    extern VkDevice device;
    extern VkQueue graphicsQueue;
    extern VkQueue presentQueue;
    extern VkSwapchainKHR swapChain;
    extern VkPhysicalDevice physicalDevice;
    extern VkSampleCountFlagBits msaaSamples;
    extern std::vector<VkImage> swapChainImages;
    extern VkFormat swapChainImageFormat;
    extern VkExtent2D swapChainExtent;
    extern std::vector<VkImageView> swapChainImageViews;
    extern std::vector<VkFramebuffer> swapChainFramebuffers;
    extern VkRenderPass renderPass;
    extern  VkDescriptorSetLayout descriptorSetLayout;
    extern VkPipelineLayout pipelineLayout;
    extern VkPipeline graphicsPipeline;
    extern VkCommandPool commandPool;
    extern VkImage colorImage;
    extern VkDeviceMemory colorImageMemory;
    extern VkImageView colorImageView;
    extern VkImage depthImage;
    extern VkDeviceMemory depthImageMemory;
    extern VkImageView depthImageView;
    extern uint32_t mipLevels;
    extern VkImage textureImage;
    extern VkDeviceMemory textureImageMemory;
    extern VkImageView textureImageView;
    extern std::vector<VkBuffer> uniformBuffers;
    extern std::vector<VkDeviceMemory> uniformBuffersMemory;
    extern std::vector<void*> uniformBuffersMapped;
    extern VkDescriptorPool descriptorPool;
    extern std::vector<VkDescriptorSet> descriptorSets;
    extern std::vector<VkCommandBuffer> commandBuffers;
    extern std::vector<VkSemaphore> imageAvailableSemaphores;
    extern std::vector<VkSemaphore> renderFinishedSemaphores;
    extern std::vector<VkFence> inFlightFences;
    extern UniformBufferObject ubo ;
    extern uint32_t currentFrame;
    //Imgui
    extern VkDescriptorPool imguiPool;
    //EditorCamera
    extern glm::vec3 cameraPosition;
    extern glm::vec3 cameraFront;
    extern glm::vec3 cameraUp;
    extern float cameraSpeed;
    extern float cameraYaw;
    extern float cameraPitch;
    extern float lastTime;
    extern float fov;
    extern bool firstMouse;
    extern bool framebufferResized;
    extern bool isPlayingGame;


    //Functions
    void start();
    void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void initVulkan();
    void initImgui();
    void mainLoop();
    void cleanupSwapChain();
    void cleanup();
    void recreateSwapChain();
    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createColorResources();
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    void createTextureImageView();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createUniformBuffers();
    void createDescriptorPool();
    
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    
    void drawFrame();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    static std::vector<char> readFile(const std::string& filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkImage createTextureImage(std::string textureName, std::string modelPath);
    VkSampler createTextureSampler();
    void createVulkanMemoryAllocator();
    bool LoadTextureFromFile(const char* filename, ImGuiTextureData* tex_data);
    void RemoveTexture(ImGuiTextureData* tex_data);
   
    void updateCamera(GLFWwindow* window, float deltaTime);
    void intersectRayWithRenderedModels(glm::vec3 rayOrigin, glm::vec3 rayDirection, int modelIndex, int& foundIntersects, glm::mat4 modelMatrix);
    void objectPicker();
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
 
        
}

//This section is a hack to get around vertex definition in DOD version of rendering
namespace Rendering {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
        bool operator==(const Vertex& other) const;

    };
}

namespace std {
    template<> struct hash<Rendering::Vertex> {
        size_t operator()(Rendering::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
