#pragma once
#include "Headers.h"

class VulkanRenderer;

class VulkanDrawable
{
public:
    explicit VulkanDrawable(VulkanRenderer *parent = nullptr);
    ~VulkanDrawable();

    void createVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);

    void prepare();

    void render();

    void destroyVertexBuffer();

public:

    // Structure storing vertex buffer metadata
    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo bufferInfo;
    } VertexBuffer;

    VkVertexInputBindingDescription viIpBind;

    VkVertexInputAttributeDescription viIpAttr[2];
private:
    std::vector<VkCommandBuffer> vecCmdDraw;

    void recordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw);
    VulkanRenderer* rendererObj;
};
