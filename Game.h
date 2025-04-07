#ifndef GAME_H
#define GAME_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    // Member variables
    uint32_t width = 800;
    uint32_t height = 600;
    std::optional<uint32_t> chosenQueueFamilyIndex;
    GLFWwindow *window;
    uint32_t imageIndex;

    // Vulkan Handles
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;
    VkPipeline graphicsPipeline;

    VkSemaphore imageAvailableSemaphore;
    VkFence imageAvailableFence;
    VkSemaphore renderFinishedSemaphore;
    VkFence syncHostWithDeviceFence;

    std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::vector<const char *> requestedDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    std::vector<const char *> requestedInstanceExtensions{};
    void initWindow();
    void initVulkan();
    void testing();
    void cleanup();
    void processInput();
    void viewInstanceExtensions();
    void viewInstanceLayers();
    void requestGLFWInstanceExtensions();
    void createInstance();
    void pickPhysicalDevice();
    void viewDeviceExtensions();
    void createLogicalDevice();
    void createSwapchain();
    void getSwapchainImages();
    void acquireNextImage();
    void createCommandPool();
    void createCommandBuffer();
    void createGraphicsPipeline();
    void draw();
    void present();
    static void printImageFormatProperties(const VkPhysicalDevice &physicalDevice, VkFormat format);
};

#endif