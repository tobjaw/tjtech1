#include "main.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ae_load_file_to_memory(const char* filename, char** result)
{
    int   size = 0;
    FILE* f    = fopen(filename, "rb");
    if (f == NULL)
    {
        *result = NULL;
        return -1;    // -1 means file opening fail
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *result = (char*) malloc(size + 1);
    if (size != fread(*result, sizeof(char), size, f))
    {
        free(*result);
        return -2;    // -2 means file reading fail
    }
    fclose(f);
    (*result)[size] = 0;
    return size;
}

void error_glfw_callback(int error, const char* description)
{
    fprintf(stderr, "Error (%d): %s\n", error, description);
}

VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                           VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                           void*                                       pUserData)
{

    fprintf(stderr, "VK Validation: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

int main(void)
{
    /***************************************************************************/
    /*                                   GLFW                                  */
    /***************************************************************************/
    /* init ********************************************************************/
    glfwSetErrorCallback(error_glfw_callback);

    printf("Compiled against GLFW %i.%i.%i\n",
           GLFW_VERSION_MAJOR,
           GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);

    printf("%s\n", glfwGetVersionString());

    if (!glfwInit())
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    printf("glfw init\n");

    /* window create **********************************************************/
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);


    /*************************************************************************/
    /*                              Vulkan Init                              */
    /*************************************************************************/


    if (!glfwVulkanSupported())
    {
        fprintf(stderr, "Vulkan missing.\n");
        exit(EXIT_FAILURE);
    }

    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT = NULL;


    /* validation layers *****************************************************/

    const char* validationLayers[] = {"VK_LAYER_LUNARG_standard_validation"};

#ifdef NDEBUG
    const bool validationLayersEnable = false;
#else
    const bool validationLayersEnable = true;
#endif

    uint32_t validationLayersAvailableCount = 0;
    vkEnumerateInstanceLayerProperties(&validationLayersAvailableCount, NULL);
    printf("found %d InstanceLayer(s)\n", validationLayersAvailableCount);

    VkLayerProperties validationLayersAvailable[validationLayersAvailableCount];
    vkEnumerateInstanceLayerProperties(&validationLayersAvailableCount, validationLayersAvailable);

    for (uint8_t i = 0; i < validationLayersAvailableCount; ++i)
    {
        VkLayerProperties layer = validationLayersAvailable[i];
        printf("  %s\n", layer.layerName);
    }

    bool    validationPossible     = true;
    uint8_t validationLayersLength = sizeof(validationLayers) / sizeof(validationLayers[0]);
    for (uint8_t i = 0; i < validationLayersLength; ++i)
    {
        bool validationLayerFound = false;
        for (uint8_t j = 0; j < validationLayersAvailableCount; ++j)
        {
            const char* layerRequired = validationLayers[i];
            const char* layer         = validationLayersAvailable[j].layerName;
            if (strcmp(layerRequired, layer) == 0)
            {
                validationLayerFound = true;
            }
        }

        if (!validationLayerFound)
        {
            validationPossible = false;
            break;
        }
    }
    printf("validation %s\n", validationPossible ? "possible" : "impossible");
    printf("validation %s\n", validationLayersEnable ? "enabled" : "disabled");


    /* app creation **********************************************************/

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Hello Triangle";
    appInfo.applicationVersion = _VK_MAKE_VERSION(1u, 0u, 0u);
    appInfo.pEngineName        = "tjtech1";
    appInfo.engineVersion      = _VK_MAKE_VERSION(1u, 0u, 0u);
    appInfo.apiVersion         = _VK_MAKE_VERSION(1u, 0u, 0u);


    /* instance creation *****************************************************/

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.flags                = 0;
    instanceCreateInfo.pApplicationInfo     = &appInfo;

    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if (validationLayersEnable)
    {
        instanceCreateInfo.enabledExtensionCount   = glfwExtensionCount + 1;
        glfwExtensions[glfwExtensionCount]         = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    }
    else
    {
        instanceCreateInfo.enabledExtensionCount   = glfwExtensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    }


    if (validationLayersEnable && !validationPossible)
    {
        fprintf(stderr, "Vulkan Validation Layers requested, but not available.\n");
        exit(EXIT_FAILURE);
    }

    /* layer injection */
    if (validationLayersEnable)
    {
        instanceCreateInfo.enabledLayerCount   = validationLayersLength;
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
    }

    PFN_vkCreateInstance pfnCreateInstance =
        (PFN_vkCreateInstance) glfwGetInstanceProcAddress(NULL, "vkCreateInstance");

    VkInstance instance;
    VkResult   instanceCreateResult;
    if ((instanceCreateResult = pfnCreateInstance(&instanceCreateInfo, NULL, &instance)) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan Instance Creation Error: %d.\n", instanceCreateResult);
        exit(EXIT_FAILURE);
    }


    /* instanceExtensions check ******************************************************/
    uint32_t instanceExtensionsAvailableCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionsAvailableCount, NULL);
    printf("found %d InstanceExtensions(s)\n", instanceExtensionsAvailableCount);

    VkExtensionProperties instanceExtensionsAvailables[instanceExtensionsAvailableCount];
    vkEnumerateInstanceExtensionProperties(
        NULL, &instanceExtensionsAvailableCount, instanceExtensionsAvailables);

    for (uint8_t i = 0; i < instanceExtensionsAvailableCount; ++i)
    {
        VkExtensionProperties ext = instanceExtensionsAvailables[i];
        printf("  %s %d\n", ext.extensionName, ext.specVersion);
    }


    /* debug extension *******************************************************/
    if (validationLayersEnable)
    {
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = vk_debug_callback;

        vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessengerEXT == NULL ||
            (vkCreateDebugUtilsMessengerEXT(
                 instance, &createInfo, NULL, &vkDebugUtilsMessengerEXT) != VK_SUCCESS))
        {
            fprintf(stderr, "failed to set up debug callback!");
        }
    }


    /* surface ***************************************************************/
    VkSurfaceKHR surface;

    VkResult surfaceCreateResult;
    if ((surfaceCreateResult = glfwCreateWindowSurface(instance, window, NULL, &surface)) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan Surface Creation Error: %d.\n", surfaceCreateResult);
        exit(EXIT_FAILURE);
    }


    /* physical device *******************************************************/
    const char*   devicePhysicalExtensionsRequired[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const uint8_t devicePhysicalExtensionsRequiredLength =
        sizeof(devicePhysicalExtensionsRequired) / sizeof(devicePhysicalExtensionsRequired[0]);

    uint32_t devicesPhysicalCount = 0;
    vkEnumeratePhysicalDevices(instance, &devicesPhysicalCount, NULL);


    if (devicesPhysicalCount == 0)
    {
        fprintf(stderr, "failed to find physical devices\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice devicesPhysical[devicesPhysicalCount];
    vkEnumeratePhysicalDevices(instance, &devicesPhysicalCount, devicesPhysical);

    printf("found %d physical device(s)\n", devicesPhysicalCount);

    /* find suitable device **************************************************/
    VkPhysicalDevice devicePhysical = VK_NULL_HANDLE;
    /* swapchain query details */
    struct SwapChainDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR*      formats;
        uint32_t                 formatsCount;
        /* VkSurfaceFormatKHR       formatSelected; */
        VkPresentModeKHR* presentModes;
        uint32_t          presentModesCount;
        /* VkPresentModeKHR         presentModeSelected; */
    } swapChainDetails;

    for (uint32_t i = 0; i < devicesPhysicalCount; ++i)
    {
        printf("  device %d:\n", i);
        VkPhysicalDevice device = devicesPhysical[i];

        /* device features */
        VkPhysicalDeviceFeatures devicePhysicalFeatures;
        vkGetPhysicalDeviceFeatures(device, &devicePhysicalFeatures);
        if (!devicePhysicalFeatures.shaderInt16)
            continue;


        /* device extensions */
        uint32_t devicePhysicalExtensionsCount = 0;
        vkEnumerateDeviceExtensionProperties(device, NULL, &devicePhysicalExtensionsCount, NULL);

        printf("    found %d physical device extension(s)\n", devicePhysicalExtensionsCount);

        VkExtensionProperties devicePhysicalExtensions[devicePhysicalExtensionsCount];
        vkEnumerateDeviceExtensionProperties(
            device, NULL, &devicePhysicalExtensionsCount, devicePhysicalExtensions);

        bool devicePhysicalExtensionsRequirementsMet = true;

        for (uint8_t i = 0; i < devicePhysicalExtensionsRequiredLength; ++i)
        {
            bool devicePhysicalExtensionFound = false;
            for (uint8_t j = 0; j < devicePhysicalExtensionsCount; ++j)
            {
                const char* extRequired = devicePhysicalExtensionsRequired[i];
                const char* ext         = devicePhysicalExtensions[j].extensionName;
                printf("      %s\n", ext);
                if (strcmp(extRequired, ext) == 0)
                {
                    devicePhysicalExtensionFound = true;
                }
            }

            if (!devicePhysicalExtensionFound)
            {
                devicePhysicalExtensionsRequirementsMet = false;
                break;
            }
        }

        if (!devicePhysicalExtensionsRequirementsMet)
            continue;


        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.capabilities);

        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &swapChainDetails.formatsCount, NULL);
        printf("    found %d format(s)\n", swapChainDetails.formatsCount);

        if (swapChainDetails.formatsCount != 0)
        {
            swapChainDetails.formats = (malloc(
                (size_t)(swapChainDetails.formatsCount * sizeof(swapChainDetails.formats))));
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device, surface, &swapChainDetails.formatsCount, swapChainDetails.formats);
        }

        for (uint32_t i = 0; i < swapChainDetails.formatsCount; ++i)
        {
            VkSurfaceFormatKHR format = swapChainDetails.formats[i];
            printf("      format=%d, colorSpace=%d\n", format.format, format.colorSpace);
        }

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &swapChainDetails.presentModesCount, NULL);
        printf("    found %d present mode(s)\n", swapChainDetails.presentModesCount);

        if (swapChainDetails.presentModesCount != 0)
        {
            swapChainDetails.presentModes = (malloc((size_t)(
                swapChainDetails.presentModesCount * sizeof(swapChainDetails.presentModes))));
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                      surface,
                                                      &swapChainDetails.presentModesCount,
                                                      swapChainDetails.presentModes);
        }

        for (uint32_t i = 0; i < swapChainDetails.presentModesCount; ++i)
        {
            VkPresentModeKHR mode = swapChainDetails.presentModes[i];
            printf("      %d\n", mode);
        }

        if (swapChainDetails.formatsCount == 0 || swapChainDetails.presentModesCount == 0)
            continue;


        /* found device */
        devicePhysical = device;
        break;
    }

    if (devicePhysical == VK_NULL_HANDLE)
    {
        fprintf(stderr, "failed to find suitable physical device\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("    suitable physical device\n");
    }


    /* swapChain config format ***********************************************/
    VkSurfaceFormatKHR swapChainConfigFormat      = {};
    bool               swapChainConfigFormatFound = false;
    printf("swapChain\n  %d available format(s)\n", swapChainDetails.formatsCount);

    if (swapChainDetails.formatsCount == 1 &&
        swapChainDetails.formats[0].format == VK_FORMAT_UNDEFINED)
    {
        swapChainConfigFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
        swapChainConfigFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapChainConfigFormatFound       = true;
    }
    else
    {
        for (uint32_t i = 0; i < swapChainDetails.formatsCount; ++i)
        {
            VkSurfaceFormatKHR formatKHR  = swapChainDetails.formats[i];
            VkFormat           format     = formatKHR.format;
            VkColorSpaceKHR    colorSpace = formatKHR.colorSpace;

            if (format == VK_FORMAT_B8G8R8A8_UNORM &&
                colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                swapChainConfigFormat      = formatKHR;
                swapChainConfigFormatFound = true;
            }
        }
    }

    if (!swapChainConfigFormatFound)
    {
        swapChainConfigFormat = swapChainDetails.formats[0];
        printf("    using fallback\n");
    }

    printf("    using format %d\n", swapChainConfigFormat.format);
    printf("    using colorSpace %d\n", swapChainConfigFormat.colorSpace);


    /* swapChain config presentMode ******************************************/
    VkPresentModeKHR swapChainConfigPresentMode      = VK_PRESENT_MODE_FIFO_KHR;    // VSYNC
    bool             swapChainConfigPresentModeFound = false;
    printf("  %d available present mode(s)\n", swapChainDetails.presentModesCount);

    for (uint32_t i = 0; i < swapChainDetails.presentModesCount; ++i)
    {
        VkPresentModeKHR modeKHR = swapChainDetails.presentModes[i];
        // prefer tripple buffering
        if (modeKHR == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            swapChainConfigPresentMode      = modeKHR;
            swapChainConfigPresentModeFound = true;
        }
    }

    for (uint32_t i = 0; i < swapChainDetails.presentModesCount; ++i)
    {
        VkPresentModeKHR modeKHR = swapChainDetails.presentModes[i];
        // NO VSYNC
        if (modeKHR == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            swapChainConfigPresentMode      = modeKHR;
            swapChainConfigPresentModeFound = true;
        }
    }

    if (!swapChainConfigPresentModeFound)
    {
        printf("    using fallback\n");
    }

    printf("    using present mode %d\n", swapChainConfigPresentMode);


    /* swapChain config swapExtent *******************************************/
    VkExtent2D swapChainConfigExtent = swapChainDetails.capabilities.currentExtent;

    if (swapChainConfigExtent.width == UINT32_MAX)
    {
        VkExtent2D swapChainConfigExtentMax = swapChainDetails.capabilities.maxImageExtent;
        VkExtent2D swapChainConfigExtentMin = swapChainDetails.capabilities.minImageExtent;

        swapChainConfigExtent.width = swapChainConfigExtentMax.width < WINDOW_WIDTH
                                          ? swapChainConfigExtentMax.width
                                          : WINDOW_WIDTH;
        swapChainConfigExtent.height = swapChainConfigExtentMax.height < WINDOW_HEIGHT
                                           ? swapChainConfigExtentMax.height
                                           : WINDOW_HEIGHT;
        swapChainConfigExtent.width = swapChainConfigExtentMin.width > swapChainConfigExtent.width
                                          ? swapChainConfigExtentMin.width
                                          : swapChainConfigExtent.width;
        swapChainConfigExtent.height =
            swapChainConfigExtentMin.height > swapChainConfigExtent.height
                ? swapChainConfigExtentMin.height
                : swapChainConfigExtent.height;
    }

    printf("  currentExtent\n    res: %dx%d\n",
           swapChainConfigExtent.width,
           swapChainConfigExtent.height);

    /* physical device queues ************************************************/
    uint32_t devicePhysicalQueueGraphicsFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        devicePhysical, &devicePhysicalQueueGraphicsFamilyCount, NULL);

    printf("found %d physical device queue families\n", devicePhysicalQueueGraphicsFamilyCount);

    VkQueueFamilyProperties
        devicePhysicalQueueGraphicsFamilies[devicePhysicalQueueGraphicsFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(devicePhysical,
                                             &devicePhysicalQueueGraphicsFamilyCount,
                                             devicePhysicalQueueGraphicsFamilies);

    int32_t devicePhysicalQueueGraphicsIndex = -1;
    for (uint32_t i = 0; i < devicePhysicalQueueGraphicsFamilyCount; ++i)
    {
        VkQueueFamilyProperties family = devicePhysicalQueueGraphicsFamilies[i];

        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            devicePhysicalQueueGraphicsIndex = i;
            break;
        }
    }
    printf("using physical device queue graphics with index %d\n",
           devicePhysicalQueueGraphicsIndex);

    int32_t devicePhysicalQueuePresentIndex = -1;
    for (uint32_t i = 0; i < devicePhysicalQueueGraphicsFamilyCount; ++i)
    {
        VkBool32 presentSupport = false;
        VkResult deviceSurfaceSupportResult;
        if ((deviceSurfaceSupportResult =
                 vkGetPhysicalDeviceSurfaceSupportKHR(
                     devicePhysical, i, surface, &presentSupport) == VK_SUCCESS))
        {
            devicePhysicalQueuePresentIndex = i;
            break;
        }
    }
    printf("using physical device queue present with index %d\n", devicePhysicalQueuePresentIndex);


    /* TODO: actually check for present queue */
    /* int32_t devicePhysicalQueuePresentIndex = devicePhysicalQueueGraphicsIndex; */


    /* device ****************************************************************/
    VkDevice device;

    /* queue info */
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex        = devicePhysicalQueueGraphicsIndex;
    deviceQueueCreateInfo.queueCount              = 1;

    float deviceQueuePriority = 1.0f;

    deviceQueueCreateInfo.pQueuePriorities = &deviceQueuePriority;

    /* device features */
    VkPhysicalDeviceFeatures deviceFeatures = {};

    /* createInfo */
    VkDeviceCreateInfo deviceCreateInfo      = {};
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount   = 1;
    deviceCreateInfo.ppEnabledExtensionNames = devicePhysicalExtensionsRequired;

    /* layer injection */
    if (validationLayersEnable)
    {
        deviceCreateInfo.enabledLayerCount   = validationLayersLength;
        deviceCreateInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    /* create */
    VkResult deviceCreateResult;
    if ((deviceCreateResult = vkCreateDevice(devicePhysical, &deviceCreateInfo, NULL, &device)) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan Device Creation Error: %d.\n", deviceCreateResult);
        exit(EXIT_FAILURE);
    }


    /* grapicsQueue **********************************************************/
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, devicePhysicalQueueGraphicsIndex, 0, &graphicsQueue);


    /* present queue *********************************************************/
    VkQueue presentQueue;
    vkGetDeviceQueue(device, devicePhysicalQueuePresentIndex, 0, &presentQueue);


    /* swapChain creation ****************************************************/
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 &&
        imageCount > swapChainDetails.capabilities.maxImageCount)
    {
        imageCount = swapChainDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface                  = surface;

    swapChainCreateInfo.minImageCount    = imageCount;
    swapChainCreateInfo.imageFormat      = swapChainConfigFormat.format;
    swapChainCreateInfo.imageColorSpace  = swapChainConfigFormat.colorSpace;
    swapChainCreateInfo.imageExtent      = swapChainConfigExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (devicePhysicalQueueGraphicsIndex != devicePhysicalQueuePresentIndex)
    {
        uint32_t pQueueFamilyIndices[2];
        pQueueFamilyIndices[0]                    = devicePhysicalQueueGraphicsIndex;
        pQueueFamilyIndices[1]                    = devicePhysicalQueuePresentIndex;
        swapChainCreateInfo.pQueueFamilyIndices   = pQueueFamilyIndices;
        swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
    }
    else
    {
        swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices   = NULL;
    }

    swapChainCreateInfo.preTransform   = swapChainDetails.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode    = swapChainConfigPresentMode;
    swapChainCreateInfo.clipped        = VK_TRUE;
    swapChainCreateInfo.oldSwapchain   = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    VkResult       swapChainCreateResult;
    if ((swapChainCreateResult =
             vkCreateSwapchainKHR(device, &swapChainCreateInfo, NULL, &swapChain)) != VK_SUCCESS)
    {
        fprintf(stderr, "swapChain creation Error: %d.\n", swapChainCreateResult);
        exit(EXIT_FAILURE);
    }


    /* swapChain Images ******************************************************/
    uint32_t swapChainImagesCount;
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImagesCount, NULL);
    VkImage swapChainImages[swapChainImagesCount];
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImagesCount, swapChainImages);


    /* swapChainImageViews ***************************************************/
    VkImageView swapChainImageViews[swapChainImagesCount];
    for (uint32_t i = 0; i < swapChainImagesCount; ++i)
    {
        VkImage swapChainImage = swapChainImages[i];

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                 = swapChainImage;

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format   = swapChainConfigFormat.format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        VkResult imageViewCreateResult;
        if ((imageViewCreateResult = vkCreateImageView(
                 device, &createInfo, NULL, &swapChainImageViews[i])) != VK_SUCCESS)
        {
            fprintf(stderr, "imageView creation Error: %d.\n", imageViewCreateResult);
            exit(EXIT_FAILURE);
        }
    }




    /*************************************************************************/
    /*                                pipeline                               */
    /*************************************************************************/
    char* vertShaderCode;
    char* fragShaderCode;

    uint32_t vertShaderCodeLength =
        ae_load_file_to_memory("/data/dev/tjtech1/shaders/vert.spv", &vertShaderCode);
    uint32_t fragShaderCodeLength =
        ae_load_file_to_memory("/data/dev/tjtech1/shaders/frag.spv", &fragShaderCode);

    if (!(vertShaderCodeLength > 0 && fragShaderCodeLength > 0))
    {
        fprintf(stderr, "shader load error\n");
        exit(EXIT_FAILURE);
    }

    struct ShaderCode
    {
        uint32_t length;
        char*    shader;
    } shaderCodes[2];

    shaderCodes[0].length = vertShaderCodeLength;
    shaderCodes[0].shader = vertShaderCode;
    shaderCodes[1].length = fragShaderCodeLength;
    shaderCodes[1].shader = fragShaderCode;

    VkShaderModule shaderModules[2];

    for (uint32_t i = 0; i < 2; ++i)
    {
        struct ShaderCode        shaderCode             = shaderCodes[i];
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
        shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = shaderCode.length;
        shaderModuleCreateInfo.pCode    = ((uint32_t*) (shaderCode.shader));

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule) !=
            VK_SUCCESS)
        {

            fprintf(stderr, "shaderModule create error\n");
            exit(EXIT_FAILURE);
        }
        shaderModules[i] = shaderModule;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = shaderModules[0];
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = shaderModules[1];
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 0;
    vertexInputInfo.pVertexBindingDescriptions      = NULL;    // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions    = NULL;    // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float) swapChainConfigExtent.width;
    viewport.height     = (float) swapChainConfigExtent.height;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent   = swapChainConfigExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;    // Optional
    rasterizer.depthBiasClamp          = 0.0f;    // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;        // Optional
    multisampling.pSampleMask           = NULL;        // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;    // Optional
    multisampling.alphaToOneEnable      = VK_FALSE;    // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;    // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;    // Optional
    colorBlending.blendConstants[1] = 0.0f;    // Optional
    colorBlending.blendConstants[2] = 0.0f;    // Optional
    colorBlending.blendConstants[3] = 0.0f;    // Optional

    VkPipelineLayout           pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;       // Optional
    pipelineLayoutInfo.pSetLayouts                = NULL;    // Optional
    pipelineLayoutInfo.pushConstantRangeCount     = 0;       // Optional
    pipelineLayoutInfo.pPushConstantRanges        = NULL;    // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS)
    {
        fprintf(stderr, "pipeline layout create error\n");
        exit(EXIT_FAILURE);
    }

    /* window check **********************************************************/
    if (!window)
    {
        /* XXX:  missing validation layer destruction  */
        /* TODO: implement deconstructor  */
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyDevice(device, NULL);
        vkDestroyInstance(instance, NULL);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    /*************************************************************************/
    /*                                  Main                                 */
    /*************************************************************************/
    /* draw loop *************************************************************/
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }


    /*************************************************************************/
    /*                                Destroy                                */
    /*************************************************************************/


    if (validationLayersEnable)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        vkDestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT");
        if (vkDestroyDebugUtilsMessengerEXT != NULL)
        {
            vkDestroyDebugUtilsMessengerEXT(instance, vkDebugUtilsMessengerEXT, NULL);
        }
    }

    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    for (uint32_t i = 0; i < 2; ++i)
    {
        VkShaderModule shaderModule = shaderModules[i];
        vkDestroyShaderModule(device, shaderModule, NULL);
    }
    for (uint32_t i = 0; i < swapChainImagesCount; ++i)
    {
        VkImageView imageView = swapChainImageViews[i];
        vkDestroyImageView(device, imageView, NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
