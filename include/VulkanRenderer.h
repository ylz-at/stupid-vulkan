#pragma once

#include "Headers.h"
#include "VulkanSwapChain.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

class VulkanRenderer {
public:
    VulkanRenderer(VulkanApplication *app, VulkanDevice *deviceObject);

    ~VulkanRenderer();

public:
    // Simple lift cycle
    void initialize();

    bool render();

    // Create an empty window
    void createPresentationWindow(const int &windowWidth = 500, const int &windowHeight = 500);

    void setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer &cmdBuf);

    //! Windows procedure method for handling events
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Destroy the presentation window
    void destroyPresentationWindow();

    // Getter functions for member variable specific to classes
    inline VulkanApplication *getApplication() { return application; }

    inline VulkanDevice *getDevice() { return deviceObj; }

    inline VulkanSwapChain *getSwapChain() { return swapChainObj; }

    void createCommandPool();

    void buildSwapChainAndDepthImage();

    void createDepthImage();

    void destroyCommandBuffer();

    void destroyCommandPool();

    void destroyDepthBuffer();

public:
#ifdef _WIN32
#define APP_NAME_STR_LEN 80
    HINSTANCE connection;
    char name[APP_NAME_STR_LEN];
    HWND window;
#else
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t *window;
    xcb_intern_atom_reply_t *reply;
#endif
    struct {
        VkFormat format;
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } Depth;

    VkCommandBuffer cmdDepthImage;
    VkCommandPool cmdPool;
    int width, height;
private:
    VulkanApplication *application;
    VulkanDevice *deviceObj;
    VulkanSwapChain *swapChainObj;
};
