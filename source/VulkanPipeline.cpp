#include "VulkanPipeline.h"
#include "VulkanApplication.h"
#include "VulkanShader.h"
#include "VulkanRenderer.h"


VulkanPipeline::VulkanPipeline() {
    appObj = VulkanApplication::GetInstance();
    deviceObj = appObj->deviceObj;
}

VulkanPipeline::~VulkanPipeline() {}

void VulkanPipeline::createPipelineCache() {
    VkResult result;

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.pNext = nullptr;
    pipelineCacheCreateInfo.initialDataSize = 0;
    pipelineCacheCreateInfo.pInitialData = nullptr;
    pipelineCacheCreateInfo.flags = 0;

    result = vkCreatePipelineCache(deviceObj->device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
    assert(result == VK_SUCCESS);
}

bool VulkanPipeline::createPipeline(VulkanDrawable *drawableObj, VkPipeline *pipeline, VulkanShader *shaderObj,
                                    VkBool32 includeDepth, VkBool32 includeVi) {
#define VK_DYNAMIC_STATE_RANGE_SIZE 30
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

    // Specify the dynamic state information to pipeline through
    // VkPipelineDynamicStateCreateInfo control struct
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.pNext = nullptr;
    vertexInputStateCreateInfo.flags = 0;
    if (includeVi) {
        vertexInputStateCreateInfo.vertexBindingDescriptionCount =
                sizeof(drawableObj->viIpBind) / sizeof(VkVertexInputBindingDescription);
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &drawableObj->viIpBind;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
                sizeof(drawableObj->viIpAttr) / sizeof(VkVertexInputAttributeDescription);
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = drawableObj->viIpAttr;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.pNext = nullptr;
    inputAssemblyStateCreateInfo.flags = 0;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo = {};
    rasterStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterStateCreateInfo.pNext = nullptr;
    rasterStateCreateInfo.flags = 0;
    rasterStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterStateCreateInfo.depthClampEnable = includeDepth;
    rasterStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterStateCreateInfo.depthBiasConstantFactor = 0;
    rasterStateCreateInfo.depthBiasClamp = 0;
    rasterStateCreateInfo.depthBiasSlopeFactor = 0;
    rasterStateCreateInfo.lineWidth = 1.0f;

    // Create the viewport state create info and provide the number of viewport and scissors being used in the rendering pipeline.
    VkPipelineColorBlendAttachmentState colorBlendAttachmentStateInfo[1] = {};
    colorBlendAttachmentStateInfo[0].colorWriteMask = 0xf;
    colorBlendAttachmentStateInfo[0].blendEnable = VK_FALSE;
    colorBlendAttachmentStateInfo[0].alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentStateInfo[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentStateInfo[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentStateInfo[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentStateInfo[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentStateInfo[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.flags = 0;
    colorBlendStateCreateInfo.pNext = nullptr;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentStateInfo;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
//    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
    colorBlendStateCreateInfo.blendConstants[0] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.pNext = nullptr;
    viewportStateCreateInfo.flags = 0;
    viewportStateCreateInfo.viewportCount = NUMBER_OF_VIEWPORTS;
    viewportStateCreateInfo.pViewports = nullptr;
    viewportStateCreateInfo.scissorCount = NUMBER_OF_SCISSORS;
    viewportStateCreateInfo.pScissors = nullptr;

    // Specify the dynamic state count and VkDynamicState enum stating which
    // dynamic state will use the values from dynamic state commands rather
    // than from the pipeline state creation info.
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.pNext = nullptr;
    depthStencilStateCreateInfo.flags = 0;
    depthStencilStateCreateInfo.depthTestEnable = includeDepth;
    depthStencilStateCreateInfo.depthWriteEnable = includeDepth;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateCreateInfo.back.compareMask = 0;
    depthStencilStateCreateInfo.back.reference = 0;
    depthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.writeMask = 0;
    depthStencilStateCreateInfo.minDepthBounds = 0;
    depthStencilStateCreateInfo.maxDepthBounds = 0;
    depthStencilStateCreateInfo.front = depthStencilStateCreateInfo.back;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.pNext = nullptr;
    multisampleStateCreateInfo.flags = 0;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.rasterizationSamples = NUM_SAMPLES;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
    multisampleStateCreateInfo.minSampleShading = 0.0;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.layout = drawableObj->pipelineLayout;;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex = 0;
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pStages = shaderObj->shaderStages;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.renderPass = appObj->rendererObj->renderPass;
    pipelineCreateInfo.subpass = 0;

    // Create the pipeline using the meta-data store in the VkGraphicsPipelineCreateInfo object
    if (vkCreateGraphicsPipelines(deviceObj->device, pipelineCache, 1, &pipelineCreateInfo, nullptr, pipeline) ==
        VK_SUCCESS) {
        return true;
    }
    return false;
}

// Destroy the pipeline cache object when no more required
void VulkanPipeline::destroyPipelineCache()
{
    vkDestroyPipelineCache(deviceObj->device, pipelineCache, nullptr);
}
