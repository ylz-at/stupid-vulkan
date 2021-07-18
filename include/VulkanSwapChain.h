#pragma once

#include "Headers.h"

class VulkanInstance;

class VulkanDevice;

class VulkanRenderer;

class VulkanApplication;

struct SwapChainBuffer {
    VkImage image;
    VkImageView view;
};

struct SwapChainPrivateVariables {
    VkSurfaceCapabilitiesKHR surfCapabilities;
    uint32_t presentModeCount;
    std::vector<VkPresentModeKHR> presentModes;
    VkExtent2D swapchainExtent;
    uint32_t desiredNumberOfSwapChainImages;
    VkSurfaceTransformFlagBitsKHR preTransform;
    VkPresentModeKHR swapchainPresentMode;
    std::vector<VkImage> swapchainImages;
    std::vector<VkSurfaceFormatKHR> surfFormats;
};

struct SwapChainPublicVariables {
    VkSurfaceKHR surface;
    uint32_t swapchainImageCount;
    VkSwapchainKHR swapChain;
    std::vector<SwapChainBuffer> colorBuffer;
    VkSemaphore presentCompleteSemaphore;
    uint32_t currentColorBuffer;
    VkFormat format;
};

class VulkanSwapChain {
public:
    VulkanSwapChain(VulkanRenderer *renderer);

    ~VulkanSwapChain();

    void initializeSwapChain();

    void createSwapChain(const VkCommandBuffer &cmd);

    void destroySwapChain();

    void setSwapChainExtent(uint32_t width, uint32_t height);

private:
    VkResult createSwapChainExtensions();

    void getSupportedFormats();

    VkResult createSurface();

    uint32_t getGraphicsQueueWithPresentationSupport();

    void getSurfaceCapabilitiesAndPresentMode();

    void managePresentMode();

    void createSwapChainColorImages();

    void createColorImageView(const VkCommandBuffer &cmd);

public:
    SwapChainPublicVariables scPublicVars;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;

private:
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkDestroySurfaceKHR fpDestroySurfaceKHR;

    // Layer Extensions Debugging
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;

    SwapChainPrivateVariables scPrivateVars;
    VulkanRenderer *rendererObj; // parent
    VulkanApplication *appObj;


};
