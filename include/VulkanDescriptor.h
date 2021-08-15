#pragma once

#include "Headers.h"

class VulkanDevice;

class VulkanApplication;

class VulkanDescriptor {
public:
    VulkanDescriptor();

    ~VulkanDescriptor();

    void createDescriptor(bool useTexture);

    void destroyDescriptor();

    // Defines the descriptor sets layout binding and create descriptor layout
    virtual void createDescriptorSetLayout(bool useTexture) = 0;

    // Destroy the valid descriptor layout object
    void destroyDescriptorLayout();

    virtual void createDescriptorPool(bool useTexture) = 0;

    void destroyDescriptorPool();

    virtual void createDescriptorResources() = 0;

    virtual void createDescriptorSet(bool useTexture) = 0;

    void destroyDescriptorSet();

    virtual void createPipelineLayout() = 0;

    void destroyPipelineLayouts();

public:
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSet;
    VulkanDevice* deviceObj;
};

