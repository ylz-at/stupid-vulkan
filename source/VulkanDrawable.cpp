#include "VulkanDrawable.h"

#include "VulkanApplication.h"
#include "VulkanRenderer.h"
#include "Wrappers.h"


VulkanDrawable::VulkanDrawable(VulkanRenderer *parent) {
    memset(&VertexBuffer, 0, sizeof(VertexBuffer));

    rendererObj = parent;

    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {};
    presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    presentCompleteSemaphoreCreateInfo.pNext = nullptr;
    presentCompleteSemaphoreCreateInfo.flags = 0;

    VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo = {};
    drawingCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    drawingCompleteSemaphoreCreateInfo.pNext = nullptr;
    drawingCompleteSemaphoreCreateInfo.flags = 0;

    VulkanDevice *deviceObj = VulkanApplication::GetInstance()->deviceObj;
    vkCreateSemaphore(deviceObj->device, &presentCompleteSemaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
    vkCreateSemaphore(deviceObj->device, &drawingCompleteSemaphoreCreateInfo, nullptr, &drawingCompleteSemaphore);

}

VulkanDrawable::~VulkanDrawable() = default;

void VulkanDrawable::destroyCommandBuffer() {
    VulkanApplication *appObj = VulkanApplication::GetInstance();
    VulkanDevice *deviceObj = appObj->deviceObj;
    for (auto &i : vecCmdDraw) {
        vkFreeCommandBuffers(deviceObj->device, rendererObj->cmdPool, 1, &i);
    }
}

void VulkanDrawable::destroySynchronizationObjects() {
    VulkanApplication *appObj = VulkanApplication::GetInstance();
    VulkanDevice *deviceObj = appObj->deviceObj;
    vkDestroySemaphore(deviceObj->device, presentCompleteSemaphore, nullptr);
    vkDestroySemaphore(deviceObj->device, drawingCompleteSemaphore, nullptr);
}

void VulkanDrawable::createVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride,
                                        bool useTexture) {
    VulkanApplication *appObj = VulkanApplication::GetInstance();
    VulkanDevice *deviceObj = appObj->deviceObj;

    VkResult result;
    bool pass;

    result = vkCreateBuffer(deviceObj->device, new VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags =0,
            .size = dataSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
    }, nullptr, &VertexBuffer.buf);
    assert(result == VK_SUCCESS);

    VkMemoryRequirements memRqrmnt;
    vkGetBufferMemoryRequirements(deviceObj->device, VertexBuffer.buf, &memRqrmnt);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = memRqrmnt.size;
    pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &allocInfo.memoryTypeIndex);
    assert(pass);

    result = vkAllocateMemory(deviceObj->device, &allocInfo, nullptr, &(VertexBuffer.mem));
    assert(result == VK_SUCCESS);
    VertexBuffer.bufferInfo.range = memRqrmnt.size;
    VertexBuffer.bufferInfo.offset = 0;

    // Map the physical device memory region to the host
    uint8_t *pData;
    result = vkMapMemory(deviceObj->device, VertexBuffer.mem, 0, memRqrmnt.size, 0, (void **) &pData);
    assert(result == VK_SUCCESS);

    // Copy the data in the mapped memory
    memcpy(pData, vertexData, dataSize);

    // Unmap the device memory
    vkUnmapMemory(deviceObj->device, VertexBuffer.mem);

    // Bind the allocated buffer resource to the device memory
    result = vkBindBufferMemory(deviceObj->device, VertexBuffer.buf, VertexBuffer.mem, 0);
    assert(result == VK_SUCCESS);

    viIpBind.binding = 0;
    viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    viIpBind.stride = dataStride;

    viIpAttr[0].binding = 0;
    viIpAttr[0].location = 0;
    viIpAttr[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    viIpAttr[0].offset = 0;
    viIpAttr[1].binding = 0;
    viIpAttr[1].location = 1;
    viIpAttr[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
    viIpAttr[1].offset = 16;

}

void VulkanDrawable::initViewports(VkCommandBuffer *cmd) {
    viewport.height = (float) rendererObj->height;
    viewport.width = (float) rendererObj->width;
    viewport.minDepth = (float) 0.0f;
    viewport.maxDepth = (float) 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(*cmd, 0, NUMBER_OF_VIEWPORTS, &viewport);
}

void VulkanDrawable::initScissors(VkCommandBuffer *cmd) {
    scissor.extent.height = rendererObj->height;
    scissor.extent.width = rendererObj->width;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(*cmd, 0, NUMBER_OF_SCISSORS, &scissor);
}

void VulkanDrawable::destroyVertexBuffer() {
    vkDestroyBuffer(rendererObj->getDevice()->device, VertexBuffer.buf, nullptr);
    vkFreeMemory(rendererObj->getDevice()->device, VertexBuffer.mem, nullptr);
}

void VulkanDrawable::recordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw) {
    VulkanDevice *deviceObj = rendererObj->getDevice();
    VkClearValue clearValues[2];
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};

    // Specify the depth/stencil clear value
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    vkCmdBeginRenderPass(*cmdDraw, new VkRenderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = rendererObj->renderPass,
            .framebuffer = rendererObj->frameBuffers[currentBuffer],
            .renderArea = VkRect2D{.offset = VkOffset2D{
                    .x = 0,
                    .y =0,
            },
                    .extent = VkExtent2D{
                            .width = static_cast<uint32_t>(rendererObj->width),
                            .height = static_cast<uint32_t>(rendererObj->height)
                    },
            },
            .clearValueCount = 2,
            .pClearValues = clearValues,

    }, VK_SUBPASS_CONTENTS_INLINE);

    // Bound the pi with the graphics pipeline
    vkCmdBindPipeline(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

    // Bind the vertex buffer
    const VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(*cmdDraw, 0, 1, &VertexBuffer.buf, offsets);

    // Bind the Index buffer
    vkCmdBindIndexBuffer(*cmdDraw, VertexIndex.idx, 0, VK_INDEX_TYPE_UINT16);

    initViewports(cmdDraw);
    initScissors(cmdDraw);

    vkCmdDraw(*cmdDraw, 3, 1, 0, 0);
    // Draw the object using indexed draw API
    vkCmdDrawIndexed(*cmdDraw, 6, 1, 0, 0, 0);

    // End of render pass instance recording
    vkCmdEndRenderPass(*cmdDraw);
}

void VulkanDrawable::prepare() {
    VulkanDevice *deviceObj = rendererObj->getDevice();
    vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size());
    // For each swapBuffer color surface image buffer
    // allocate the corresponding command buffer
    for (int i = 0; i < rendererObj->getSwapChain()->scPublicVars.colorBuffer.size(); i++) {
        // Allocate, create and start command buffer recording
        CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &vecCmdDraw[i]);
        CommandBufferMgr::beginCommandBuffer(vecCmdDraw[i]);

        // Create the render pass instance
        recordCommandBuffer(i, &vecCmdDraw[i]);

        // Finish the command buffer recording
        CommandBufferMgr::endCommandBuffer(vecCmdDraw[i]);
    }
}

void VulkanDrawable::render() {
    VulkanDevice *deviceObj = rendererObj->getDevice();
    VulkanSwapChain *swapChainObj = rendererObj->getSwapChain();

    uint32_t &currentColorImage = swapChainObj->scPublicVars.currentColorBuffer;
    VkSwapchainKHR &swapChain = swapChainObj->scPublicVars.swapChain;

    VkFence nullFence = VK_NULL_HANDLE;

    VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain, UINT64_MAX,
                                                          presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.commandBufferCount = (uint32_t) sizeof(&vecCmdDraw[currentColorImage]) / sizeof(VkCommandBuffer);
    submitInfo.pCommandBuffers = &vecCmdDraw[currentColorImage];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;


    // Queue the command buffer for execution
    CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], &submitInfo);
    swapChainObj->fpQueuePresentKHR(deviceObj->queue, new VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &drawingCompleteSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &currentColorImage,
            .pResults = nullptr,
    });
    assert(result == VK_SUCCESS);
}

void VulkanDrawable::createVertexIndex(const void *indexData, uint32_t dataSize, uint32_t dataStride) {
    VulkanApplication *appObj = VulkanApplication::GetInstance();
    VulkanDevice *deviceObj = appObj->deviceObj;

    VkResult result;
    bool pass;

    // Create the Buffer resource metadata information
    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = nullptr;
    bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufInfo.size = dataSize;
    bufInfo.queueFamilyIndexCount = 0;
    bufInfo.pQueueFamilyIndices = nullptr;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.flags = 0;

    // Create the Buffer resource
    result = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &VertexIndex.idx);
    assert(result == VK_SUCCESS);

    // Get the Buffer resource requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceObj->device, VertexIndex.idx, &memRequirements);

    // Create memory allocation metadata information
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = memRequirements.size;

    // Get the compatible type of memory
    pass = deviceObj->memoryTypeFromProperties(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &allocInfo.memoryTypeIndex);
    assert(pass);

    // Allocate the physical baking for buffer resource
    result = vkAllocateMemory(deviceObj->device, &allocInfo, nullptr, &(VertexIndex.mem));
    assert(result == VK_SUCCESS);
    VertexIndex.bufferInfo.range = memRequirements.size;
    VertexIndex.bufferInfo.offset = 0;

    // Map the physical device memory region to the host
    uint8_t *pData;
    result = vkMapMemory(deviceObj->device, VertexIndex.mem, 0, memRequirements.size, 0, (void **) &pData);
    assert(result == VK_SUCCESS);

    // Copy the data in the mapped memory
    memcpy(pData, indexData, dataSize);

    // Unmap the device memory
    vkUnmapMemory(deviceObj->device, VertexIndex.mem);

    // Bind the allocated buffer resource to the device memory
    result = vkBindBufferMemory(deviceObj->device, VertexIndex.idx, VertexIndex.mem, 0);
    assert(result == VK_SUCCESS);
}

void VulkanDrawable::destroyVertexIndex() {
    vkDestroyBuffer(rendererObj->getDevice()->device, VertexIndex.idx, nullptr);
    vkFreeMemory(rendererObj->getDevice()->device, VertexIndex.mem, nullptr);

}
