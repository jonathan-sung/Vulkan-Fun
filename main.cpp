#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <vector>

std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char *> requestedDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<const char *> requestedInstanceExtensions{};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    // Vulkan Handles
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

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

    // get glfw required extensions
    auto requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::cout << "required extensions:" << std::endl;
    for (uint32_t i = 0; i < extensionCount; i++)
    {
        std::cout << "\t" << requiredExtensions[i] << std::endl;
        requestedInstanceExtensions.push_back(requiredExtensions[i]);
    }

    // Set up Vulkan Instance

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "app name";
    applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    applicationInfo.pEngineName = "engine name";
    applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = validationLayers.size();
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = requestedInstanceExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requestedInstanceExtensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance");

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

    uint32_t deviceExtensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());
    std::cout << "available device extensions: " << std::endl;
    for (const VkExtensionProperties &currentExtension : availableDeviceExtensions)
    {
        std::cout << "\t" << currentExtension.extensionName << std::endl;
    }

    // Create logical device

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    std::optional<uint32_t> chosenQueueFamilyIndex;
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
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDestroyDevice(logicalDevice, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}