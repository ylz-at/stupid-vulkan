#include "VulkanApplication.h"
#include "VulkanDrawable.h"

std::unique_ptr<VulkanApplication> VulkanApplication::instance;
std::once_flag VulkanApplication::onlyOnce;

extern std::vector<const char *> instanceExtensionNames;
extern std::vector<const char *> layerNames;
extern std::vector<const char *> deviceExtensionNames;

VulkanApplication::VulkanApplication() {
    // At application start up, enumerate instance layers
    instanceObj.layerExtension.getInstanceLayerProperties();

    deviceObj = nullptr;
    debugFlag = false;
    rendererObj = nullptr;
    isPrepared = false;
    isResizing = false;
}

VulkanApplication::~VulkanApplication() {
    delete rendererObj;
    rendererObj = nullptr;
}

// Returns the Singleton object of VulkanApplication
VulkanApplication *VulkanApplication::GetInstance() {
    std::call_once(onlyOnce, []() { instance.reset(new VulkanApplication()); });
    return instance.get();
}

VkResult VulkanApplication::createVulkanInstance(std::vector<const char *> &layers,
                                                 std::vector<const char *> &extensions, const char *appName) {
    return instanceObj.createInstance(layers, extensions, appName);
}

// This function is responsible for creating the logical device.
// The process of logical device creation requires the following steps:-
// 1. Get the physical device specific layer and corresponding extensions [Optional]
// 2. Create user define VulkanDevice object
// 3. Provide the list of layer and extension that needs to enabled in this physical device
// 4. Get the physical device or GPU properties
// 5. Get the memory properties from the physical device or GPU
// 6. Query the physical device exposed queues and related properties
// 7. Get the handle of graphics queue
// 8. Create the logical device, connect it to the graphics queue.

// High level function for creating device and queues
VkResult VulkanApplication::handShakeWithDevice(VkPhysicalDevice *gpu, std::vector<const char *> &layers,
                                                std::vector<const char *> &extensions) {
    // The user define Vulkan Device object this will manage the
    // Physical and Logical device and their queue and properties
    deviceObj = new VulkanDevice(gpu);
    if (!deviceObj) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    // Print the devices available layer and their extension.
    deviceObj->layerExtension.getDeviceExtensionProperties(gpu);

    // Get the physical device or GPU properties.
    vkGetPhysicalDeviceProperties(*gpu, &deviceObj->gpuProps);
    printf("vendor id %04x, device name %s\n", &deviceObj->gpuProps.vendorID, &deviceObj->gpuProps.deviceName);

    // Get the memory properties from the physical device or GPU.
    vkGetPhysicalDeviceMemoryProperties(*gpu, &deviceObj->memoryProps);

    // Query the available queues on the physical device and their properties.
    deviceObj->getPhysicalDeviceQueueAndProperties();

    // Retrieve the Queue which support graphics pipeline.
    deviceObj->getGraphicsQueueHandle();

    // Create Logical Device, ensure that this device is connected to graphics queue.
    return deviceObj->createDevice(layers, extensions);
}

VkResult VulkanApplication::enumeratePhysicalDevices(std::vector<VkPhysicalDevice> &gpus) {
    uint32_t gpuDeviceCount;

    VkResult result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, nullptr);
    assert(result == VK_SUCCESS);
    assert(gpuDeviceCount);

    gpus.resize(gpuDeviceCount);

    result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, gpus.data());
    assert(result == VK_SUCCESS);

    return result;
}

void VulkanApplication::initialize() {
    char title[] = "Hello World!!!";

    // Check if the supplied layer are support or not
    if (debugFlag) {
        instanceObj.layerExtension.areLayersSupported(layerNames);
    }

    // Create the Vulkan instance with specified layer and extension names.
    createVulkanInstance(layerNames, instanceExtensionNames, title);

    // Create the debugging report if debugging is enabled
    if (debugFlag) {
        instanceObj.layerExtension.createDebugReportCallback();
    }

    // Get the list of physical devices on the system, there might be multiple Physical GPUs.
    enumeratePhysicalDevices(gpus);

    // This example use only one device which is available first.
    if (!gpus.empty()) {
        handShakeWithDevice(&gpus[0], layerNames, deviceExtensionNames);
    }

    if (!rendererObj) {
        rendererObj = new VulkanRenderer(this, deviceObj);

        // Create an empty window 500x500
        rendererObj->createPresentationWindow(500, 500);

        // Initialize swapChain
        rendererObj->getSwapChain()->initializeSwapChain();
    }
    rendererObj->initialize();
}

void VulkanApplication::resize() {
    // If prepared then only proceed for
    if (!isPrepared) {
        return;
    }

    isResizing = true;

    vkDeviceWaitIdle(deviceObj->device);
    rendererObj->destroyFramebuffers();
    rendererObj->destroyCommandPool();
    rendererObj->destroyPipeline();
    rendererObj->getPipelineObject()->destroyPipelineCache();
    for (VulkanDrawable *drawableObj : *rendererObj->getDrawingItems()) {
        drawableObj->destroyDescriptor();
    }
    rendererObj->destroyRenderpass();
    rendererObj->getSwapChain()->destroySwapChain();
    rendererObj->destroyDrawableVertexBuffer();
    rendererObj->destroyDrawableUniformBuffer();
    rendererObj->destroyDepthBuffer();
    rendererObj->initialize();
    prepare();

    isResizing = false;
}

void VulkanApplication::deInitialize() {
    rendererObj->destroyPipeline();
    rendererObj->getPipelineObject()->destroyPipelineCache();
    for (VulkanDrawable *drawableObj : *rendererObj->getDrawingItems()) {
        drawableObj->destroyDescriptor();
    }
    rendererObj->getShader()->destroyShaders();
    rendererObj->destroyFramebuffers();
    rendererObj->destroyRenderpass();
    rendererObj->destroyDrawableVertexBuffer();
    rendererObj->destroyDrawableUniformBuffer();
    rendererObj->destroyDrawableCommandBuffer();
    rendererObj->destroyDepthBuffer();
    rendererObj->getSwapChain()->destroySwapChain();
    rendererObj->destroyCommandBuffer();
    rendererObj->destroyDrawableSynchronizationObjects();
    rendererObj->destroyCommandPool();
    rendererObj->destroyPresentationWindow();

    deviceObj->destroyDevice();
    if (debugFlag) {
        instanceObj.layerExtension.destroyDebugReportCallback();
    }
    instanceObj.destroyInstance();
}

void VulkanApplication::prepare() {
    isPrepared = false;
    rendererObj->prepare();
    isPrepared = true;
}

void VulkanApplication::update() {
    rendererObj->update();
}

bool VulkanApplication::render() {
    // Place holder, this will be utilized in the upcoming chapters
    if (!isPrepared) {
        return false;
    }
    return rendererObj->render();
}


