//
// Created by Arouay on 02/04/2023.
//

#ifndef DRAFT_VK_COMMANDBUFFERS_HPP
#define DRAFT_VK_COMMANDBUFFERS_HPP

#include <vulkan/vulkan_core.h>
#include <vector>
#include "Vertex.hpp"

namespace dvk {

    class CommandBuffers {
    private:
        VkCommandPool commandPool{};
        std::vector<VkCommandBuffer> commandBuffers;
        VkPhysicalDevice* physicalDevice;
        VkDevice* device;
        VkSurfaceKHR* surface;
        std::vector<VkFramebuffer>* swapchainFramebuffers;
        VkRenderPass* renderPass;
        VkExtent2D* swapChainExtent;
        VkPipeline* graphicsPipeline;
        VkBuffer* vertexBuffer;
        std::vector<Vertex>* vertices;

        void createCommandBuffers();
        void createCommandPool();
    public:
        CommandBuffers(
                VkPhysicalDevice* physicalDevice,
                VkDevice* device,
                VkSurfaceKHR* surface,
                std::vector<VkFramebuffer>* swapchainFramebuffers,
                VkRenderPass* renderPass,
                VkExtent2D* swapChainExtent,
                VkPipeline* graphicsPipeline,
                VkBuffer* vertexBuffer,
                std::vector<Vertex>* vertices
                );

        ~CommandBuffers();
        std::vector<VkCommandBuffer>* getCommandBuffer();

        void recordCommandBuffer(int currentFrame, uint32_t imageIndex);
    };

} // dvk

#endif //DRAFT_VK_COMMANDBUFFERS_HPP
