#include "VulkanDescriptor.h"
#include "VulkanApplication.h"

VulkanDescriptor::VulkanDescriptor() {
    deviceObj = VulkanApplication::GetInstance()->deviceObj;
}

VulkanDescriptor::~VulkanDescriptor() {
}

void VulkanDescriptor::createDescriptor(bool useTexture) {
    // Create the uniform buffer resource
    createDescriptorResources();

    // Create the descriptor pool and
    // use it for descriptor set allocation
    createDescriptorPool(useTexture);

    // Create descriptor set with uniform buffer data in it
    createDescriptorSet(useTexture);
}

void VulkanDescriptor::destroyDescriptor() {
    destroyDescriptorLayout();
    destroyPipelineLayouts();
    destroyDescriptorSet();
    destroyDescriptorPool();
}

void VulkanDescriptor::destroyDescriptorLayout() {
    for (auto &i : descLayout) {
        vkDestroyDescriptorSetLayout(deviceObj->device, i, nullptr);
    }
    descLayout.clear();
}

void VulkanDescriptor::destroyPipelineLayouts() {
    vkDestroyPipelineLayout(deviceObj->device, pipelineLayout, nullptr);
}

void VulkanDescriptor::destroyDescriptorPool() {
    vkDestroyDescriptorPool(deviceObj->device, descriptorPool, nullptr);
}

void VulkanDescriptor::destroyDescriptorSet() {
    vkFreeDescriptorSets(deviceObj->device, descriptorPool, (uint32_t) descriptorSet.size(), &descriptorSet[0]);
}