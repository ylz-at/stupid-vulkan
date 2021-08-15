#pragma once

#include "Headers.h"
#include "VulkanDescriptor.h"
#include "Wrappers.h"

class VulkanRenderer;

class VulkanDrawable : public VulkanDescriptor {
public:
    explicit VulkanDrawable(VulkanRenderer *parent = nullptr);

    ~VulkanDrawable();

    void createVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);

    void createVertexIndex(const void *indexData, uint32_t dataSize, uint32_t dataStride);

    void prepare();

    void render();

    void update();

    void initViewports(VkCommandBuffer *cmd);

    void initScissors(VkCommandBuffer *cmd);

    void initPushConstant(VkCommandBuffer *cmd);

    void setPipeline(VkPipeline *vulkanPipeline) { pipeline = vulkanPipeline; }

    VkPipeline *getPipeline() { return pipeline; }

    void createUniformBuffer();

    void createDescriptorPool(bool useTexture);

    void createDescriptorResources();

    void createDescriptorSet(bool useTexture);

    void createDescriptorSetLayout(bool useTexture);

    void createPipelineLayout();

    void destroyVertexBuffer();

    void destroyVertexIndex();

    void destroyCommandBuffer();

    void destroySynchronizationObjects();

    void destroyUniformBuffer();

public:

    struct {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorBufferInfo bufferInfo; // Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
        VkMemoryRequirements memoryRequirements;
        std::vector<VkMappedMemoryRange> mappedRange;
        uint8_t *pData;
    } UniformData;

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

    glm::mat4 Projection;
    glm::mat4 View;
    glm::mat4 Model;
    glm::mat4 MVP;
};
