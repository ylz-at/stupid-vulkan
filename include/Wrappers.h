#pragma once
#include "Headers.h"

/***********************COMMAND BUFFER WRAPPERS************************/

class CommandBufferMgr {
public:
    static void allocCommandBuffer(const VkDevice *device, const VkCommandPool cmdPool, VkCommandBuffer *cmdBuf,
                                   const VkCommandBufferAllocateInfo *commandBufferInfo = nullptr);

    static void beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo *inCmdBufferInfo = nullptr);

    static void endCommandBuffer(VkCommandBuffer cmdBuf);

    static void submitCommandBuffer(const VkQueue &queue, const VkCommandBuffer *cmdBufList,
                                    const VkSubmitInfo *submitInfo = nullptr, const VkFence &fence = VK_NULL_HANDLE);

};
