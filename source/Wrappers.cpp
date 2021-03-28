#include "Wrappers.h"

void CommandBufferMgr::allocCommandBuffer(const VkDevice *device, const VkCommandPool cmdPool, VkCommandBuffer *cmdBuf,
                                          const VkCommandBufferAllocateInfo *commandBufferInfo) {
    VkResult result;

    if (commandBufferInfo) {
        result = vkAllocateCommandBuffers(*device, commandBufferInfo, cmdBuf);
        assert(!result);
        return;
    }

    VkCommandBufferAllocateInfo cmdInfo = {};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.pNext = nullptr;
    cmdInfo.commandPool = cmdPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof (VkCommandBuffer);

    result = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
    assert(!result);
}

void CommandBufferMgr::beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo *inCmdBufferInfo) {
    VkResult result;
    if (inCmdBufferInfo) {
        result = vkBeginCommandBuffer(cmdBuf, inCmdBufferInfo);
        assert(result);
        return;
    }

    VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
    cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cmdBufInheritInfo.pNext = nullptr;
    cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
    cmdBufInheritInfo.subpass = 0;
    cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
    cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
    cmdBufInheritInfo.queryFlags = 0;
    cmdBufInheritInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;
    cmdBufInfo.flags=0;
    cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;
    result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

    assert(result == VK_SUCCESS);
}

void CommandBufferMgr::endCommandBuffer(VkCommandBuffer cmdBuf) {
    VkResult result;
    result = vkEndCommandBuffer(cmdBuf);
    assert(result == VK_SUCCESS);
}

void CommandBufferMgr::submitCommandBuffer(const VkQueue &queue, const VkCommandBuffer *cmdBufList,
                                           const VkSubmitInfo *inSubmitInfo, const VkFence &fence) {
    VkResult result;

    if (inSubmitInfo) {
        result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
        assert(!result);
        result = vkQueueWaitIdle(queue);
        assert(!result);
        return;
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = (uint32_t) sizeof(cmdBufList) / sizeof(VkCommandBuffer);
    submitInfo.pCommandBuffers = cmdBufList;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    result = vkQueueSubmit(queue, 1, &submitInfo, fence);
    assert(!result);

    result = vkQueueWaitIdle(queue);
    assert(!result);
}