//
// Created by Arouay on 03/04/2023.
//

#include "Core.hpp"
#include <chrono>
#include <memory>
#include <iomanip>

namespace dvk::Core {
    Core::Core() :
            window(std::make_unique<Window>()),
            instance(std::make_unique<Instance>()),
            debug(std::make_unique<Debug>(instance->getInstance())),
            surface(std::make_unique<Surface>(window->getRawWindow(), instance->getInstance())),
            device(std::make_unique<Device>(instance->getInstance(), surface->getSurface())),
            swapchain(
                    std::make_unique<Swapchain>(
                            window->getRawWindow(),
                            surface->getSurface(),
                            device->getPhysicalDevice(),
                            device->getDevice()
                            )
            ),
            swapchainImageViews(
                    std::make_unique<SwapchainImageViews>(
                            device->getDevice(),
                            swapchain->getSwapchainImages(),
                            swapchain->getSwapchainImageFormat()
                            )
            ),
            renderPass(std::make_unique<RenderPass>(device->getDevice(), swapchain->getSwapchainImageFormat())),
            graphicsPipeline(
                    std::make_unique<GraphicsPipeline>(
                            device->getDevice(),
                            renderPass->getRenderPass(),
                            swapchain->getSwapchainExtent()
                            )
            ),
            framebuffers(
                    std::make_unique<Framebuffers>(
                            device->getDevice(),
                            swapchainImageViews->getSwapchainImageViews(),
                            renderPass->getRenderPass(),
                            swapchain->getSwapchainExtent()
                            )
            ),
            vertexBuffer(
                    std::make_unique<VertexBuffer>(
                            device->getDevice(),
                            device->getPhysicalDevice(),
                            device->getGraphicsQueue(),
                            surface->getSurface()
                            )
            ),
            commandBuffers(
                    std::make_unique<CommandBuffers>(
                            device->getPhysicalDevice(),
                            device->getDevice(),
                            surface->getSurface(),
                            framebuffers->getFramebuffers(),
                            renderPass->getRenderPass(),
                            swapchain->getSwapchainExtent(),
                            graphicsPipeline->getGraphicsPipeline(),
                            vertexBuffer->getVertexBuffer(),
                            vertexBuffer->getVertices()
                            )
            ),
            synchronization(
                    std::make_unique<Synchronization>(
                            device->getDevice(),
                            swapchain->getSwapchainImages(),
                            device->getGraphicsQueue(),
                            MAX_FRAMES_IN_FLIGHT
                            )
            )
    {

    }

    void Core::drawFrame()
    {
//        auto start = std::chrono::high_resolution_clock::now();

        vkWaitForFences(*(device->getDevice()), 1, &(*(synchronization->getInFlightFences()))[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(*(device->getDevice()), *(swapchain->getSwapChain()), UINT64_MAX, (*(synchronization->getImageAvailableSemaphores()))[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->recreateSwapchain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(*(device->getDevice()), 1, &(*(synchronization->getInFlightFences()))[currentFrame]);

        commandBuffers->recordCommandBuffer(currentFrame, imageIndex);

        VkSemaphore waitSemaphores[] = {(*(synchronization->getImageAvailableSemaphores()))[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphore[] = {(*(synchronization->getRenderFinishedSemaphores()))[currentFrame]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(*(commandBuffers->getCommandBuffer()))[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphore;

        if (vkQueueSubmit(*(device->getGraphicsQueue()), 1, &submitInfo, (*(synchronization->getInFlightFences()))[currentFrame]) != VK_SUCCESS){
            throw std::runtime_error("Failed to submit graphics queue!");
        }

        VkSwapchainKHR swapChains[] = {*(swapchain->getSwapChain())};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pWaitSemaphores = signalSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        presentInfo.waitSemaphoreCount = 1;

        result = vkQueuePresentKHR(*(device->getPresentationQueue()), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->isFramebufferResized()) {
            window->setFramebufferResized(false);
            this->recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

//        auto end = std::chrono::high_resolution_clock::now();
//        auto time_taken = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//        std::cout << "Frame drawn - time taken: " << time_taken << " " << "microseconds" << std::endl;
    }

    void Core::init() {

    }

    void Core::start() {
        this->window->startLoop([this](){
//            std::cout << "frame draw" << std::endl;
            this->drawFrame();
        }, true);
    }

    void Core::recreateSwapchain() {
        auto start = std::chrono::high_resolution_clock::now();

        int width = 0, height = 0;
        glfwGetFramebufferSize(window->getRawWindow(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window->getRawWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(*(device->getDevice()));

        commandBuffers.reset();
        framebuffers.reset();
        swapchainImageViews.reset();
        swapchain.reset();

        swapchain = std::make_unique<Swapchain>(
                window->getRawWindow(),
                surface->getSurface(),
                device->getPhysicalDevice(),
                device->getDevice()
                );
        swapchainImageViews = std::make_unique<SwapchainImageViews>(
                device->getDevice(),
                swapchain->getSwapchainImages(),
                swapchain->getSwapchainImageFormat()
                );
        framebuffers = std::make_unique<Framebuffers>(
                device->getDevice(),
                swapchainImageViews->getSwapchainImageViews(),
                renderPass->getRenderPass(),
                swapchain->getSwapchainExtent()
                );
        commandBuffers = std::make_unique<CommandBuffers>(
                device->getPhysicalDevice(),
                device->getDevice(),
                surface->getSurface(),
                framebuffers->getFramebuffers(),
                renderPass->getRenderPass(),
                swapchain->getSwapchainExtent(),
                graphicsPipeline->getGraphicsPipeline(),
                vertexBuffer->getVertexBuffer(),
                vertexBuffer->getVertices()
                );

        auto end = std::chrono::high_resolution_clock::now();
        auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Swapchain recreated - time taken: " << time_taken << " " << "milliseconds" << std::endl;
    }

} // dvk