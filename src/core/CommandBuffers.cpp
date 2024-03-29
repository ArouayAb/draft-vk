//
// Created by Arouay on 02/04/2023.
//

#include "CommandBuffers.hpp"
#include "QueueFamilyIndices.hpp"

namespace dvk {

    CommandBuffers::CommandBuffers(
                VkPhysicalDevice* physicalDevice,
                VkDevice* device,
                VkSurfaceKHR* surface,
                std::vector<VkFramebuffer>* swapchainFramebuffers,
                VkRenderPass* renderPass,
                VkExtent2D* swapChainExtent,
                VkPipeline* graphicsPipeline,
                VkBuffer* vertexBuffer,
                std::vector<Vertex>* vertices
            ) :
            physicalDevice(physicalDevice),
            device(device),
            surface(surface),
            swapchainFramebuffers(swapchainFramebuffers),
            renderPass(renderPass),
            swapChainExtent(swapChainExtent),
            graphicsPipeline(graphicsPipeline),
            vertexBuffer(vertexBuffer),
            vertices(vertices)
    {
        createCommandPool();
        createCommandBuffers();
//        for (int i = 0; i < commandBuffers.size(); i++) {
//            recordCommandBuffer(i, i);
//        }
    }

    CommandBuffers::~CommandBuffers() {
        vkDestroyCommandPool(*device, commandPool, nullptr);
    }

    void CommandBuffers::createCommandPool()
{
        QueueFamilyIndices queueFamilyIndices(physicalDevice, surface);

        VkCommandPoolCreateInfo commandPoolInfos{};
        commandPoolInfos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfos.queueFamilyIndex = queueFamilyIndices.getGraphicsFamilyValue();
        commandPoolInfos.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(*device, &commandPoolInfos, nullptr, &commandPool) != VK_SUCCESS){
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void CommandBuffers::createCommandBuffers()
    {
        commandBuffers.resize(swapchainFramebuffers->size());

        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool = commandPool;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(*device, &commandBufferAllocInfo, commandBuffers.data()) != VK_SUCCESS){
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    std::vector<VkCommandBuffer> *CommandBuffers::getCommandBuffer() {
        return &commandBuffers;
    }

    void CommandBuffers::recordCommandBuffer(int currentFrame, uint32_t imageIndex) {
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[currentFrame], &cmdBufferBeginInfo) != VK_SUCCESS){
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.framebuffer = (*swapchainFramebuffers)[imageIndex];
        renderPassBeginInfo.renderPass = *renderPass;
        renderPassBeginInfo.pClearValues = &clearColor;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = *swapChainExtent;
        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent->width);
        viewport.height = static_cast<float>(swapChainExtent->height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = *swapChainExtent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {*vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffers[currentFrame], static_cast<uint32_t>(vertices->size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[currentFrame]);

        if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS){
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
} // dvk