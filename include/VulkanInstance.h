#pragma once

#include "VulkanLED.h"

//Define class here
class VulkanInstance {
public:
    VulkanInstance()= default;
    ~VulkanInstance()= default;

    // VulkanInstance member variables
    VkInstance instance{};

    // Vulkan instance specific layer and extensions
    VulkanLayerAndExtension layerExtension;

    // VulkanInstance public functions
    VkResult
    createInstance(std::vector<const char *> &layers, std::vector<const char *> &extensionNames, const char *appName);

    void destroyInstance();
};


