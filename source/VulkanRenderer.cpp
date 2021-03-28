//
// Created by Jinesi on 2021/3/7.
//

#include "VulkanRenderer.h"
#include "VulkanApplication.h"
#include "Wrappers.h"


VulkanRenderer::VulkanRenderer(VulkanApplication *app, VulkanDevice *deviceObject) {
    assert(application != nullptr);
    assert(deviceObj != nullptr);

    memset(&Depth, 0, sizeof(Depth));
    memset(&connection, 0, sizeof(HINSTANCE));

    application = app;
    deviceObj = deviceObject;
    swapChainObj = new VulkanSwapChain(this);
}

VulkanRenderer::~VulkanRenderer() {
    delete swapChainObj;
    swapChainObj = nullptr;
}

void VulkanRenderer::initialize() {
    // Create an empty window 500x500
    createPresentationWindow(500, 500);

    // Initialize swapchain
    swapChainObj->initializeSwapChain();

    // We need command buffers, so create a command buffer pool
    createCommandPool();

    // Let's create the swap chain color images and depth image
    buildSwapChainAndDepthImage();
}

bool VulkanRenderer::render() {
    MSG msg; // message
    PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
    if (msg.message == WM_QUIT) {
        return false;
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);
    RedrawWindow(window, nullptr, nullptr, RDW_INTERNALPAINT);
    return true;
}

#ifdef _WIN32

// MS-Windows event handling function:
LRESULT CALLBACK VulkanRenderer::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    VulkanApplication *appObj = VulkanApplication::GetInstance();
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

void VulkanRenderer::createPresentationWindow(const int &windowWidth, const int &windowHeight) {
#ifdef _WIN32
    width = windowWidth;
    height = windowHeight;
    assert(width > 0 || height > 0);

    WNDCLASSEX winInfo;
    sprintf(name, "SwapChain presentation window");
    memset(&winInfo, 0, sizeof(WNDCLASSEX));
    winInfo.cbSize = sizeof(WNDCLASSEX);
    winInfo.style = CS_HREDRAW | CS_VREDRAW;
    winInfo.lpfnWndProc = WndProc;
    winInfo.cbClsExtra = 0;
    winInfo.cbWndExtra = 0;
    winInfo.hInstance = connection;
    winInfo.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    winInfo.hCursor = LoadIcon(nullptr, IDC_ARROW);
    winInfo.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    winInfo.lpszMenuName = nullptr;
    winInfo.lpszClassName = name;
    winInfo.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

    // Register window class
    if (!RegisterClassEx(&winInfo)) {
        // It didn't work, so try to give a useful error;
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class;
    RECT wr = {0, 0, width, height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    window = CreateWindowEx(0, name, name, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU, 100, 100, wr.right - wr.left,
                            wr.bottom - wr.top, nullptr, nullptr, connection, nullptr);
    if (!window) {
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }

    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR) &application);
#else
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int src;

    connection = xcb_connect(NULL, &src);
    if(connection == nullptr){
        std::cout << "Cannot find a compatible Vulkan ICD.\n";
        exit(-1);
    }
    setup = xcb_get_setup(connection);
    iter = xcb_setup_roots_iterator(setup);
    while(src-- > 0){
        xcb_screen_next(&iter);
    }
    screen = iter.data;
#endif
}

void VulkanRenderer::destroyPresentationWindow() {
    DestroyWindow(window);
}

#else
void VulkanRenderer::createPresentationWindow()
{
    assert(width > 0);
    assert(height > 0);

    uint32_t value_mask, value_list[32];

    window = xcb_generate_id(connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, width, height, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
    reply = xcb_intern_atom_reply(connection, cookie2, 0);

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1,	&(*reply).atom);
    free(reply);

    xcb_map_window(connection, window);

    // Force the x/y coordinates to 100,100 results are identical in consecutive runs
    const uint32_t coords[] = { 100,  100 };
    xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
    xcb_flush(connection);

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(connection))) {
        if ((e->response_type & ~0x80) == XCB_EXPOSE)
            break;
    }
}

void VulkanRenderer::destroyWindow()
{
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);
}

#endif // _WIN32

void VulkanRenderer::createCommandPool() {
    VulkanDevice *obj = application->deviceObj;
    VkResult res;

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext = nullptr;
    cmdPoolInfo.queueFamilyIndex = obj->graphicsQueueWithPresentIndex;
    cmdPoolInfo.flags = 0;

    res = vkCreateCommandPool(obj->device, &cmdPoolInfo, nullptr, &cmdPool);
    assert(res == VK_SUCCESS);
}

void VulkanRenderer::createDepthImage() {
    VkResult result;
    bool pass;
    VkImageCreateInfo imageInfo = {};

    // If the depth format is undefined, use fallback as 16-byte value
    if (Depth.format == VK_FORMAT_UNDEFINED) {
        Depth.format = VK_FORMAT_D16_UNORM;
    }

    const VkFormat depthFormat = Depth.format;

    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(*deviceObj->gpu, depthFormat, &props);
    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    } else if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    } else {
        std::cout << "Unsupported Depth Format, try other Depth formats.\n";
        exit(-1);
    }
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = depthFormat;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = NUM_SAMPLES;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.flags = 0;

    // User create image info and create the image objects
    result = vkCreateImage(deviceObj->device, &imageInfo, nullptr, &Depth.image);
    assert(result == VK_SUCCESS);
    VkMemoryRequirements memRqrmnt;
    vkGetImageMemoryRequirements(deviceObj->device, Depth.image, &memRqrmnt);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.pNext = nullptr;
    memAlloc.allocationSize = 0;
    memAlloc.memoryTypeIndex = 0;
    memAlloc.allocationSize = memRqrmnt.size;
    pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, 0, &memAlloc.memoryTypeIndex);
    assert(pass);

    // Allocate the memory for image objects
    result = vkAllocateMemory(deviceObj->device, &memAlloc, nullptr, &Depth.mem);
    assert(result == VK_SUCCESS);

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.pNext = nullptr;
    imgViewInfo.image = VK_NULL_HANDLE;
    imgViewInfo.format = depthFormat;
    imgViewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.baseArrayLayer = 0;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.flags = 0;

    if (depthFormat == VK_FORMAT_D16_UNORM_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
        depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        imgViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    // Use command buffer to create the depth image. This includes -
    // Command buffer allocation, recording with begin/end scope and submission.
    CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImage);
    CommandBufferMgr::beginCommandBuffer(cmdDepthImage);
    {
        setImageLayout(Depth.image, imgViewInfo.subresourceRange.aspectMask, VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits) 0, cmdDepthImage);
    }
    CommandBufferMgr::endCommandBuffer(cmdDepthImage);
    CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDepthImage);

    imgViewInfo.image = Depth.image;
    result = vkCreateImageView(deviceObj->device, &imgViewInfo, nullptr, &Depth.view);
    assert(result == VK_SUCCESS);
}

void VulkanRenderer::destroyDepthBuffer()
{
    vkDestroyImageView(deviceObj->device, Depth.view, nullptr);
    vkDestroyImage(deviceObj->device, Depth.image, nullptr);
    vkFreeMemory(deviceObj->device, Depth.mem, nullptr);
}

void VulkanRenderer::destroyCommandBuffer()
{
    VkCommandBuffer cmdBufs[] = { cmdDepthImage };
    vkFreeCommandBuffers(deviceObj->device, cmdPool, sizeof(cmdBufs)/sizeof(VkCommandBuffer), cmdBufs);
}

void VulkanRenderer::destroyCommandPool()
{
    vkDestroyCommandPool(application->deviceObj->device, cmdPool, nullptr);
}

void VulkanRenderer::buildSwapChainAndDepthImage() {
    deviceObj->getDeviceQueue();
    swapChainObj->createSwapChain(cmdDepthImage);
    createDepthImage();
}

void VulkanRenderer::setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout,
                                    VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask,
                                    const VkCommandBuffer &cmdBuf) {
    // Dependency on cmd
    assert(cmdBuf != VK_NULL_HANDLE);

    // The deviceObj->queue must be initialized
    VkImageMemoryBarrier imgMemoryBarrier = {};
    imgMemoryBarrier.sType			= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgMemoryBarrier.pNext			= NULL;
    imgMemoryBarrier.srcAccessMask	= srcAccessMask;
    imgMemoryBarrier.dstAccessMask	= 0;
    imgMemoryBarrier.oldLayout		= oldImageLayout;
    imgMemoryBarrier.newLayout		= newImageLayout;
    imgMemoryBarrier.image			= image;
    imgMemoryBarrier.subresourceRange.aspectMask	= aspectMask;
    imgMemoryBarrier.subresourceRange.baseMipLevel	= 0;
    imgMemoryBarrier.subresourceRange.levelCount	= 1;
    imgMemoryBarrier.subresourceRange.layerCount	= 1;

    if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
    }

    VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlagBits destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    vkCmdPipelineBarrier(cmdBuf, srcStages, destStages, 0, 0, nullptr, 0, nullptr, 1, &imgMemoryBarrier);
}

