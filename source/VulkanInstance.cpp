#include "VulkanInstance.h"

VkResult VulkanInstance::createInstance(std::vector<const char *> &layers, std::vector<const char *> &extensionNames,
                                        const char *appName) {
    layerExtension.appRequestedExtensionNames = extensionNames;
    layerExtension.appRequestedLayerNames = layers;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = appName;
    appInfo.engineVersion = 1;

    // VK_API_VERSION is now deprecated, use VK_MAKE_VERSION instead
    appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);

    // Define the Vulkan instance create info structure
    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = &layerExtension.dbgReportCreateInfo;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;

    // Specify the list of layer name to be enabled
    instInfo.enabledLayerCount = (uint32_t) layers.size();
    instInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    // Specify the list of extensions to be used in the application;
    instInfo.enabledExtensionCount = (uint32_t) extensionNames.size();
    instInfo.ppEnabledExtensionNames = extensionNames.empty() ? nullptr : extensionNames.data();

    VkResult result = vkCreateInstance(&instInfo, nullptr, &instance);
    assert(result == VK_SUCCESS);

    return result;

}

void VulkanInstance::destroyInstance() {
    vkDestroyInstance(instance, nullptr);
}
