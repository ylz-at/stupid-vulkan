#pragma once

#include "Headers.h"

class VulkanRenderer;

class VulkanDrawable {
public:
    explicit VulkanDrawable(VulkanRenderer *parent = nullptr);

    ~VulkanDrawable();

    void createVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
    void createVertexIndex(const void *indexData, uint32_t dataSize, uint32_t dataStride);

    void prepare();

    void render();

    void initViewports(VkCommandBuffer *cmd);

    void initScissors(VkCommandBuffer *cmd);

    void setPipeline(VkPipeline *vulkanPipeline) { pipeline = vulkanPipeline; }

    VkPipeline *getPipeline() { return pipeline; }

    void destroyVertexBuffer();
    void destroyVertexIndex();

    void destroyCommandBuffer();

    void destroySynchronizationObjects();

public:

    // Structure storing vertex buffer metadata
    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
    } VertexBuffer;

    struct {
        VkBuffer idx;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
    } VertexIndex;

    VkVertexInputBindingDescription viIpBind;

    VkVertexInputAttributeDescription viIpAttr[2];
private:
    std::vector<VkCommandBuffer> vecCmdDraw;

    void recordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw);

    VkViewport viewport;
    VkRect2D scissor;
    VkSemaphore presentCompleteSemaphore;
    VkSemaphore drawingCompleteSemaphore;
    VulkanRenderer *rendererObj;
    VkPipeline *pipeline;
};
