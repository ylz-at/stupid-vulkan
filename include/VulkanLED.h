#pragma once
#include <Headers.h>

struct LayerProperties {
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> extensions;
};

class VulkanLayerAndExtension {
public:
    VulkanLayerAndExtension();
    ~VulkanLayerAndExtension();

    /******* LAYER AND EXTENSION MEMBER FUNCTION AND VARAIBLES *******/
    // List of layer and extension names requested by the application.
    std::vector<const char *> appRequestedLayerNames;
    std::vector<const char *> appRequestedExtensionNames;

    // Layers and corresponding extension list
    std::vector<LayerProperties> layerPropertyList;

    // Instance/global layer
    VkResult getInstanceLayerProperties();

    // Global extensions
    VkResult getExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice *gpu = nullptr);

    // Device based extensions
    VkResult getDeviceExtensionProperties(VkPhysicalDevice *gpu);

    /******* VULKAN DEBUGGING MEMBER FUNCTION AND VARAIBLES *******/
    VkBool32 areLayersSupported(std::vector<const char *> &layerNames);
    VkResult createDebugReportCallback();
    void destroyDebugReportCallback();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugFunction(VkFlags msgFlags,
                                                        VkDebugReportObjectTypeEXT objType,
                                                        uint64_t srcObject,
                                                        size_t location,
                                                        int32_t msgCode,
                                                        const char *layerPrefix,
                                                        const char *msg,
                                                        void *userData);

private:
    PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;
    VkDebugReportCallbackEXT debugReportCallback;
public:
    // init with type otherwise validation will fail
    VkDebugReportCallbackCreateInfoEXT dbgReportCreateInfo = {.sType =  VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
};