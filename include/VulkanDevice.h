#pragma once
#include "VulkanLED.h"

class VulkanDevice{
public:
    VulkanDevice(VkPhysicalDevice* gpu);
    ~VulkanDevice();

public:
    VkDevice device; // Logical device
    VkPhysicalDevice* gpu; // Physical device
    VkPhysicalDeviceProperties gpuProps; // Physical device attributes
    VkPhysicalDeviceMemoryProperties memoryProps;

    // Queue
    VkQueue queue;
    std::vector<VkQueueFamilyProperties> queueFamilyProps;
    // Store all queue families exposed by the physical device. attributes
    uint32_t graphicsQueueIndex; // Stores graphics queue index
    uint32_t graphicsQueueWithPresentIndex; // Number of queue family exposed by device
    uint32_t queueFamilyCount; // Device specific layer and extensions

    // Layer and extensions
    VulkanLayerAndExtension layerExtension;

    VkResult createDevice(std::vector<const char *> & layers, std::vector<const char *> & extensions);
    void destroyDevice();

    // Get the available queues exposed by the physical devices
    void getPhysicalDeviceQueueAndProperties();

    // Query physical device to retrieve queue properties
    uint32_t getGraphicsQueueHandle();

    void getDeviceQueue();
};
