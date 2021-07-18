#pragma once

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"

class VulkanApplication {
private:
    // CTOR: Application constructor responsible for layer enumeration.
    VulkanApplication();

public:
    // DTOR
    ~VulkanApplication();

private:
    // Variable for Singleton implementation
    static std::unique_ptr<VulkanApplication> instance;
    static std::once_flag onlyOnce;

private:
    bool debugFlag;
    std::vector<VkPhysicalDevice> gpus;

public:
    // Vulkan Instance object
    VulkanInstance instanceObj;
    VulkanDevice *deviceObj;
    VulkanRenderer *rendererObj;
    bool isPrepared;
    bool isResizing;

    static VulkanApplication *GetInstance();

    // Simple program life cycle
    void initialize();

    void prepare();

    void update();

    void resize();

    bool render();

    void deInitialize();

private:
    // Create the vulkan instance object
    VkResult createVulkanInstance(std::vector<const char *> &layers, std::vector<const char *> &extensions,
                                  const char *appName);

    VkResult handShakeWithDevice(VkPhysicalDevice *gpu, std::vector<const char *> &layers,
                                 std::vector<const char *> &extensions);

    VkResult enumeratePhysicalDevices(std::vector<VkPhysicalDevice> &gpus);
};
