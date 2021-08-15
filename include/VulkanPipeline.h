#pragma once

#include "Headers.h"
class VulkanShader;
class VulkanDrawable;
class VulkanDevice;
class VulkanApplication;

// While creating the pipeline the number of viewports and number of scissors.
#define NUMBER_OF_VIEWPORTS 1
#define NUMBER_OF_SCISSORS NUMBER_OF_VIEWPORTS

class VulkanPipeline {
public:
    VulkanPipeline();
    ~VulkanPipeline();

    // Creates the pipeline cache object and stores pipeline object
    void createPipelineCache();

    // Returns the created pipeline object, it takes the drawable object which contains the vertex input rate and data interpretation information,
    // shader files, boolean flag checking enabled depth, and flag to check if the vertex input are available.
    bool
    createPipeline(VulkanDrawable *drawableObj, VkPipeline *pipeline, VulkanShader *shaderObj, VkBool32 includeDepth,
                   VkBool32 includeVi = true);

    // Destruct the pipeline cache object
    void destroyPipelineCache();

public:
    // Pipeline preparation member variables
    // Pipeline cache object
    VkPipelineCache pipelineCache;
    VulkanApplication* appObj;
    VulkanDevice* deviceObj;
};
