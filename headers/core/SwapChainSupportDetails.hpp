#ifndef SWAPCHAINSUPPORTDETAILS
#define SWAPCHAINSUPPORTDETAILS

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>

namespace dvk {

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

    public:
        void querySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
    };
}

#endif