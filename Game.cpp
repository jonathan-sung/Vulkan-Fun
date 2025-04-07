#include "Game.h"

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

Game::Game()
{
    std::cout << "Game object successfully created!" << std::endl;
    initWindow();
    initVulkan();
}
Game::~Game()
{
    vkDeviceWaitIdle(device);
    cleanup();
    glfwTerminate();
    std::cout << "Game object successfully destroyed!" << std::endl;
}

void Game::run()
{
    std::clog << "Game running..." << std::endl;
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
    createInstance();
    // createSurface();
    pickPhysicalDevice();
    viewDeviceExtensions();
    createLogicalDevice();
    createSwapchain();
    // createImageViews();
    // createRenderPass();
    // createGraphicsPipeline();
    // createFramebuffers();
    createCommandPool();
    createCommandBuffer();
    // createSyncObjects();
    testing();
}

void Game::testing()
{
    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.size = 32;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // bufferInfo.queueFamilyIndexCount = 1;
    // bufferInfo.pQueueFamilyIndices = &chosenQueueFamilyIndex.value();
    vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

    // Find memory type which can be accessed by both CPU and GPU
    std::optional<uint32_t> chosenMemoryTypeIndex;
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        uint32_t requiredFlags = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bool hasMemoryTypeAllFlags = (bool)((memoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags);

        std::clog << "Memory Type Index " << i << ":" << std::endl;
        std::clog << "\tFlag Bits: \t" << std::bitset<32>(memoryProperties.memoryTypes[i].propertyFlags) << std::endl;
        std::clog << "\tIs coherent:"
                  << ": \t" << std::boolalpha << hasMemoryTypeAllFlags << std::endl;
        std::clog << std::endl;

        if (hasMemoryTypeAllFlags)
        {
            chosenMemoryTypeIndex = i;
            // break;
        }
    }

    // Allocate memory block
    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = 64; // 64 bytes to hold 16 uint32_t variables
    allocateInfo.memoryTypeIndex = chosenMemoryTypeIndex.value();

    VkDeviceMemory memoryBlock;

    vkAllocateMemory(device, &allocateInfo, nullptr, &memoryBlock);

    // Bind memory to buffer resource
    VkMemoryRequirements bufferMemoryRequirements{};
    vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);
    std::clog << "Buffer Memory Requirements:" << std::endl;
    std::clog << "\t Alignment: " << bufferMemoryRequirements.alignment << std::endl;
    std::clog << "\t Size: " << bufferMemoryRequirements.size << std::endl;
    std::clog << "\t Memory Types: \t" << std::bitset<32>(bufferMemoryRequirements.memoryTypeBits) << std::endl;
    vkBindBufferMemory(device, buffer, memoryBlock, 0);

    void *bufferAddress;
    vkMapMemory(device, memoryBlock, 0, VK_WHOLE_SIZE, 0, &bufferAddress);

    // start command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdFillBuffer(commandBuffer, buffer, 0, bufferMemoryRequirements.size, 42);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(queue, 1, &submitInfo, nullptr);

    vkDeviceWaitIdle(device);

    std::clog << *(uint32_t *)bufferAddress << std::endl;

    vkUnmapMemory(device, memoryBlock);
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, memoryBlock, nullptr);
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
    // Look at instance information
    viewInstanceExtensions();
    viewInstanceLayers();
    requestGLFWInstanceExtensions();
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
    if (!chosenQueueFamilyIndex.has_value())
        throw std::runtime_error("no queue available!");

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
    getSwapchainImages();
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

void Game::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
}

void Game::createGraphicsPipeline()
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding.binding = 0;

    VkDescriptorSetLayout descriptorSetLayout;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &descriptorSetLayoutBinding;
    vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);

    VkPushConstantRange pushConstantRange{};

    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
    vkCreateGraphicsPipelines(device, nullptr, 1, &graphicsPipelineInfo, nullptr, &graphicsPipeline);
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

void Game::printImageFormatProperties(const VkPhysicalDevice &physicalDevice, VkFormat format)
{
    VkImageFormatProperties imageFormatProperties;
    vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &imageFormatProperties);
    std::cout << "Image Format Properties:\n";
    std::cout << "\tMax Array Layers: " << imageFormatProperties.maxArrayLayers << std::endl;
    std::cout << "\tMax Extent: {" << imageFormatProperties.maxExtent.width << ", " << imageFormatProperties.maxExtent.height << ", " << imageFormatProperties.maxExtent.depth << "}" << std::endl;
    std::cout << "\tMax Mip Levels: " << imageFormatProperties.maxMipLevels << std::endl;
    std::cout << "\tMax Resource Size: " << imageFormatProperties.maxResourceSize << " bytes, " << (imageFormatProperties.maxResourceSize / 1000) / 1000 << "MB" << std::endl;
    std::cout << "\tMax Sample Counts: " << imageFormatProperties.sampleCounts << std::endl;
}
