#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice *physicalDevice) {
    gpu = physicalDevice;
}

VulkanDevice::~VulkanDevice() = default;

// Note: This function requires queue object to be in existence before
VkResult VulkanDevice::createDevice(std::vector<const char *> &layers, std::vector<const char *> &extensions) {
    layerExtension.appRequestedLayerNames = layers;
    layerExtension.appRequestedExtensionNames = extensions;

    // Create Device with available queue information.
    VkResult result;
    float queuePriorities[1] = {0.0};
    VkDeviceQueueCreateInfo qcInfo = {};
    qcInfo.queueFamilyIndex = graphicsQueueIndex; // update by getGraphicsQueueHandle()
    qcInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qcInfo.pNext = nullptr;
    qcInfo.queueCount = 1;
    qcInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo dcInfo = {};
    dcInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dcInfo.pNext = nullptr;
    dcInfo.queueCreateInfoCount = 1;
    dcInfo.pQueueCreateInfos = &qcInfo;
    dcInfo.enabledLayerCount = 0;
    dcInfo.ppEnabledLayerNames = nullptr;
    dcInfo.enabledExtensionCount = (uint32_t)extensions.size();
    dcInfo.ppEnabledExtensionNames = extensions.empty()? nullptr: extensions.data();
    dcInfo.pEnabledFeatures = nullptr;

    result = vkCreateDevice(*gpu, &dcInfo, nullptr, &device);
    assert(result == VK_SUCCESS);
    return result;
}

void  VulkanDevice::getPhysicalDeviceQueueAndProperties() {
    // Queue queue families count with pass NULL as second parameter
    vkGetPhysicalDeviceQueueFamilyProperties2(*gpu, &queueFamilyCount, nullptr);

    // Allocate space to accommodate Queue Properties
    queueFamilyProps.resize(queueFamilyCount);

    // Get queue family properties
    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueFamilyProps.data());
}

uint32_t VulkanDevice::getGraphicsQueueHandle() {
    //	1. Get the number of Queues supported by the Physical device
    //	2. Get the properties each Queue type or Queue Family
    //			There could be 4 Queue type or Queue families supported by physical device -
    //			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT
    //			Compute Queue	- VK_QUEUE_COMPUTE_BIT
    //			DMA				- VK_QUEUE_TRANSFER_BIT
    //			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
    //	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

    bool found = false;
    // 1.
    for (unsigned int i = 0; i < queueFamilyCount; i++) {
        // 2.
        if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // 3.
            graphicsQueueIndex = i;
            found = true;
            break;
        }
    }

    assert(found);
    return graphicsQueueIndex;
}

void VulkanDevice::getDeviceQueue(){
    vkGetDeviceQueue(device, graphicsQueueWithPresentIndex, 0, &queue);
}

void VulkanDevice::destroyDevice() {
    vkDestroyDevice(device, nullptr);
}