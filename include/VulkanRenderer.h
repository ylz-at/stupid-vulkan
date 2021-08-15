#pragma once

#include "Headers.h"
#include "VulkanSwapChain.h"
#include "VulkanDrawable.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

class VulkanRenderer {
public:
    VulkanRenderer(VulkanApplication *app, VulkanDevice *deviceObject);

    ~VulkanRenderer();

public:
    // Simple lift cycle
    void initialize();

    void prepare();

    bool render();

    void update();

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

    inline std::vector<VulkanDrawable *> *getDrawingItems() { return &drawableList; }

    inline VkCommandPool *getCommandPool() { return &cmdPool; }

    inline VulkanShader *getShader() { return &shaderObj; }

    inline VulkanPipeline *getPipelineObject() { return &pipelineObj; }

    void createCommandPool();

    void buildSwapChainAndDepthImage();

    void createDepthImage();

    void createVertexBuffer();

    void createShaders();

    void createRenderPass(bool includeDepth, bool clear = true);

    void createFrameBuffer(bool includeDepth);

    void createPipelineStateManagement();

    void createDescriptors();

    void destroyCommandBuffer();

    void destroyCommandPool();

    void destroyDepthBuffer();

    void destroyDrawableVertexBuffer();

    void destroyRenderpass();

    void destroyFramebuffers();

    void destroyPipeline();

    void destroyDrawableCommandBuffer();

    void destroyDrawableSynchronizationObjects();

    void destroyDrawableUniformBuffer();

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
    VkCommandBuffer cmdVertexBuffer;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> frameBuffers; // Number of frame Buffers corresponding to each swap chain
    std::vector<VkPipeline *> pipelineList; // List of pipelines
    int width, height;
private:
    VulkanApplication *application;
    VulkanDevice *deviceObj;
    VulkanSwapChain *swapChainObj;
    std::vector<VulkanDrawable *> drawableList;
    VulkanShader shaderObj;
    VulkanPipeline pipelineObj;
};
