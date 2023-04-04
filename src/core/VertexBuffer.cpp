//
// Created by Arouay on 04/04/2023.
//

#include "VertexBuffer.hpp"

namespace dvk {
    VertexBuffer::VertexBuffer(VkDevice* device, VkPhysicalDevice* physicalDevice) : device(device), physicalDevice(physicalDevice) {
        vertices = {
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5}, {0.0f, 0.0f, 1.0f}}
        };
        createVertexBuffer();
        allocateMemory();
        fillMemory();
    }

    VertexBuffer::VertexBuffer(VkDevice* device, VkPhysicalDevice* physicalDevice, std::vector<Vertex> vertices) :
        device(device),
        physicalDevice(physicalDevice),
        vertices(vertices)
    {
        vertices = {
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5}, {0.0f, 0.0f, 1.0f}}
        };
        createVertexBuffer();
        allocateMemory();
        fillMemory();
    }

    VertexBuffer::~VertexBuffer() {
        vkDestroyBuffer(*device, vertexBuffer, nullptr);
        vkFreeMemory(*device, vertexBufferMemory, nullptr);
    }

    void VertexBuffer::createVertexBuffer() {
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Vertex) * vertices.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }
    }

    uint32_t VertexBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VertexBuffer::allocateMemory() {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(*device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(*device, vertexBuffer, vertexBufferMemory, 0);
    }

    void VertexBuffer::fillMemory() {
        void* data;
        vkMapMemory(*device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferInfo.size);
        vkUnmapMemory(*device, vertexBufferMemory);
    }

    VkBuffer* VertexBuffer::getVertexBuffer() {
        return &vertexBuffer;
    }

    std::vector<Vertex>* VertexBuffer::getVertices() {
        return &vertices;
    }
} // dvk