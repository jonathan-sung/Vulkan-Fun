#include "Game.h"

#include <iostream>

Game::Game()
{
    std::cout << "Game object successfully created!" << std::endl;
    initWindow();
    initVulkan();
}
Game::~Game()
{
    cleanup();
    glfwTerminate();
    std::cout << "Game object successfully destroyed!" << std::endl;
}

void Game::run()
{
    std::clog << "game running..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // acquireNextImage();
        // draw();
        // present();
        processInput();
        glfwPollEvents();
    }
}

void Game::initVulkan()
{
    viewInstanceExtensions();
    viewInstanceLayers();
    requestGLFWInstanceExtensions();
    createInstance();
    pickPhysicalDevice();
    viewDeviceExtensions();
    createLogicalDevice();
    createSwapchain();
    getSwapchainImages();
    createCommandPool();
}

void Game::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
}

void Game::viewInstanceExtensions()
{
    // get instance extension properties
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::cout << extensionCount << " extensions supported\n";
    std::cout << "available instance extensions:\n";
    for (const VkExtensionProperties &extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << "\n";
    }
}

void Game::viewInstanceLayers()
{
    // get instance layer properties
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    std::cout << "available layers:" << std::endl;
    for (const VkLayerProperties &layerProperty : layerProperties)
    {
        std::cout << "\t" << layerProperty.layerName << std::endl;
    }
}

void Game::requestGLFWInstanceExtensions()
{
    uint32_t extensionCount;
    // get glfw required extensions
    auto requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::cout << "required extensions:" << std::endl;
    for (uint32_t i = 0; i < extensionCount; i++)
    {
        std::cout << "\t" << requiredExtensions[i] << std::endl;
        requestedInstanceExtensions.push_back(requiredExtensions[i]);
    }
}

void Game::createInstance()
{
    // Set up Vulkan Instance

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "app name";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName = "engine name";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requestedInstanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = requestedInstanceExtensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance");
}
void Game::pickPhysicalDevice()
{
    // Look at physical devices
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    std::cout << "list of physical devices available:" << std::endl;
    for (const VkPhysicalDevice &currentPhysicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(currentPhysicalDevice, &physicalDeviceProperties);
        std::cout << "\t" << physicalDeviceProperties.deviceName << std::endl;

        // choose physical device
        if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            physicalDevice = currentPhysicalDevice;
    }

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    std::cout << "Chosen Physical Device: " << physicalDeviceProperties.deviceName << std::endl;
}

void Game::viewDeviceExtensions()
{
    uint32_t deviceExtensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());
    std::cout << "available device extensions: " << std::endl;
    for (const VkExtensionProperties &currentExtension : availableDeviceExtensions)
    {
        std::cout << "\t" << currentExtension.extensionName << std::endl;
    }
}

void Game::createLogicalDevice()
{

    // Create logical device

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    uint32_t i = 0;
    glfwCreateWindowSurface(instance, window, nullptr, &surface);
    for (const auto &currentQueueFamilyProperties : queueFamilyProperties)
    {
        VkBool32 presentationSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupported);
        if ((currentQueueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentationSupported)
        {
            chosenQueueFamilyIndex = i;
            break;
        }
    }
    if (chosenQueueFamilyIndex.has_value())
        std::cout << "Queue family found. Index: " << chosenQueueFamilyIndex.value() << std::endl;
    else
        std::cout << "queue family NOT found!" << std::endl;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = chosenQueueFamilyIndex.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = { queueCreateInfo };

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requestedDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    vkGetDeviceQueue(device, chosenQueueFamilyIndex.value(), 0, &queue);
}

void Game::createSwapchain()
{

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> availableSurfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, availableSurfaceFormats.data());

    std::cout << "available surface formats: " << std::endl;
    for (auto currentSurfaceFormat : availableSurfaceFormats)
    {
        std::cout << "\t format name: " << currentSurfaceFormat.format << std::endl;
        std::cout << "\t color space: " << currentSurfaceFormat.format << std::endl
                  << std::endl;
    }

    // Create swapchain
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = 4;
    swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = VkExtent2D{ width, height };
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // swapchainInfo.queueFamilyIndexCount = 1;
    // swapchainInfo.pQueueFamilyIndices = &chosenQueueFamilyIndex.value();
    swapchainInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
}
void Game::getSwapchainImages()
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
}

void Game::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = chosenQueueFamilyIndex.value();
    vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool);
}

void Game::acquireNextImage()
{
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, imageAvailableFence, &imageIndex);
}

void Game::draw()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(queue, 1, &submitInfo, syncHostWithDeviceFence);
}

void Game::present()
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(queue, &presentInfo);
}

void Game::cleanup()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
}

void Game::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
