#include "main.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    /*                                GLFW Init                                */
    /***************************************************************************/


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
    printf("validationLayersAvailableCount: %d\n", validationLayersAvailableCount);

    VkLayerProperties validationLayersAvailable[validationLayersAvailableCount];
    vkEnumerateInstanceLayerProperties(&validationLayersAvailableCount, validationLayersAvailable);

    for (uint8_t i = 0; i < validationLayersAvailableCount; ++i)
    {
        printf("%s %d\n",
               validationLayersAvailable[i].layerName,
               validationLayersAvailable[i].specVersion);
    }

    bool    validationPossible     = true;
    uint8_t validationLayersLength = sizeof(validationLayers) / sizeof(validationLayers[0]);
    for (uint8_t i = 0; i < validationLayersLength; ++i)
    {
        bool validationLayerFound = false;
        for (uint8_t j = 0; j < validationLayersAvailableCount; ++j)
        {
            if (strcmp(validationLayers[i], validationLayersAvailable[j].layerName) == 0)
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
    printf("validation possible: %d\n", validationPossible);
    printf("validation enabled: %d\n", validationLayersEnable);


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


    /* extensions check ******************************************************/

    uint32_t extensionsAvailableCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionsAvailableCount, NULL);
    printf("extensionsAvailableCount: %d\n", extensionsAvailableCount);

    VkExtensionProperties extensionsAvailables[extensionsAvailableCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionsAvailableCount, extensionsAvailables);

    for (uint8_t i = 0; i < extensionsAvailableCount; ++i)
    {
        printf(
            "%s %d\n", extensionsAvailables[i].extensionName, extensionsAvailables[i].specVersion);
    }


    /* PFN_vkCreateDevice pfnCreateDevice = */
    /*     (PFN_vkCreateDevice) glfwGetInstanceProcAddress(instance, "vkCreateDevice"); */


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


    /* physical device *******************************************************/

    uint32_t devicesPhysicalCount = 0;
    vkEnumeratePhysicalDevices(instance, &devicesPhysicalCount, NULL);

    printf("found %d physical device(s)\n", devicesPhysicalCount);

    if (devicesPhysicalCount == 0)
    {
        fprintf(stderr, "failed to find physical devices\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice devicesPhysical[devicesPhysicalCount];
    vkEnumeratePhysicalDevices(instance, &devicesPhysicalCount, devicesPhysical);

    VkPhysicalDevice devicePhysical = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < devicesPhysicalCount; ++i)
    {

        VkPhysicalDeviceProperties devicePhysicalProperties;
        VkPhysicalDeviceFeatures   devicePhysicalFeatures;
        vkGetPhysicalDeviceProperties(devicesPhysical[i], &devicePhysicalProperties);
        vkGetPhysicalDeviceFeatures(devicesPhysical[i], &devicePhysicalFeatures);

        if (devicePhysicalFeatures.shaderInt16)
        {
            devicePhysical = devicesPhysical[i];
            break;
        }
    }

    if (devicePhysical == VK_NULL_HANDLE)
    {
        fprintf(stderr, "failed to find suitable physical device\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("found suitable physical device\n");
    }


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
        /* VkBool32 presentSupport = false; */
        /* vkGetPhysicalDeviceSurfaceSupportKHR(devicePhysical, i, surface, &presentSupport); */
        if (devicePhysicalQueueGraphicsFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            devicePhysicalQueueGraphicsIndex = i;
            break;
        }
    }

    printf("using physical device queue with index %d\n", devicePhysicalQueueGraphicsIndex);


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
    VkDeviceCreateInfo deviceCreateInfo   = {};
    deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos    = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures     = &deviceFeatures;

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


    /*************************************************************************/
    /*                            Surface & Window                           */
    /*************************************************************************/
    /* surface ***************************************************************/
    VkSurfaceKHR surface;

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    VkResult surfaceCreateResult;
    if ((surfaceCreateResult = glfwCreateWindowSurface(instance, window, NULL, &surface)) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan Surface Creation Error: %d.\n", surfaceCreateResult);
        exit(EXIT_FAILURE);
    }

    /* TODO: actually check for present queue */
    int32_t devicePhysicalQueuePresentIndex = devicePhysicalQueueGraphicsIndex;

    /* present queue *********************************************************/
    VkQueue presentQueue;
    vkGetDeviceQueue(device, devicePhysicalQueuePresentIndex, 0, &presentQueue);

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


    /* draw loop *************************************************************/

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }


    /* destruction ***********************************************************/

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
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
