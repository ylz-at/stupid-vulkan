#include "VulkanDrawable.h"

#include "VulkanApplication.h"
#include "VulkanRenderer.h"
#include "Wrappers.h"


VulkanDrawable::VulkanDrawable(VulkanRenderer *parent) {
    memset(&VertexBuffer, 0, sizeof(VertexBuffer));

    rendererObj = parent;
}

VulkanDrawable::~VulkanDrawable() {}

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

void VulkanDrawable::destroyVertexBuffer() {
    vkDestroyBuffer(rendererObj->getDevice()->device, VertexBuffer.buf, nullptr);
    vkFreeMemory(rendererObj->getDevice()->device, VertexBuffer.mem, nullptr);
}

void VulkanDrawable::recordCommandBuffer(int currentBuffer, VkCommandBuffer *cmdDraw) {
    VkClearValue clearValues[2];
    switch (currentBuffer) {
        case 0:
            clearValues[0].color = {1.0f, 0.0f, 0.0f, 0.0f};
            break;

        case 1:
            clearValues[0].color = {0.0f, 1.0f, 0.0f, 0.0f};
            break;

        case 2:
            clearValues[0].color = {0.0f, 0.0f, 1.0f, 0.0f};
            break;

        default:
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
            break;
    }

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

    vkCmdEndRenderPass(*cmdDraw);
}

void VulkanDrawable::prepare() {
    VulkanDevice *deviceObj = rendererObj->getDevice();
    vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size());
    // For each swapbuffer color surface image buffer
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

    VkSemaphore presentCompleteSemaphore;
    vkCreateSemaphore(deviceObj->device, new VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    }, nullptr, &presentCompleteSemaphore);

    Sleep(1);

    VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain, UINT64_MAX,
                                                          presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);
    // Queue the command buffer for execution
    CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], nullptr);
    swapChainObj->fpQueuePresentKHR(deviceObj->queue, new VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &currentColorImage,
    });
    assert(result == VK_SUCCESS);

    vkDestroySemaphore(deviceObj->device, presentCompleteSemaphore, nullptr);
}