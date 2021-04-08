#pragma once
#include "Headers.h"

class VulkanShader
{
public:
    VulkanShader(){}
    ~VulkanShader(){}

    void buildShaderModuleWithSPV(uint32_t *vertShaderText, size_t vertexSPVSize, uint32_t *fragShaderText,
                               size_t fragmentSPVSize);

    void destroyShaders();

#ifdef AUTO_COMPILE_GLSL_TO_SPV
    bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *pshader, std::vector<unsigned int> &spirv)

    void buildShader(const char *vertShaderText, const char *fragShaderText)

    EShLanguage getLanguage(const VkShaderStageFlagBits shaderType)

    void initializeResources(TBuildInResource &Resource);
#endif

    VkPipelineShaderStageCreateInfo shaderStages[2];
};