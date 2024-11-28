#pragma once

#include "Model.h"
#include <glm/glm.hpp>

constexpr static unsigned int NUM_REED = 1 << 6;
constexpr static float REED_MIN_HEIGHT = 10.3f;
constexpr static float REED_MAX_HEIGHT = 16.5f;
constexpr static float REED_MIN_WIDTH = 0.28f;
constexpr static float REED_MAX_WIDTH = 0.32f;
constexpr static float REED_MIN_BEND = 3.6f;
constexpr static float REED_MAX_BEND = 4.6f;

struct ReedVertex {
	glm::vec4 pos;
	glm::vec4 normal;
};

struct Reed {
    // Position and direction
    glm::vec4 v0;
    // Bezier point and height
    glm::vec4 v1;
    // Physical model guide and width
    glm::vec4 v2;
    // Up vector and stiffness coefficient
    glm::vec4 up;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Reed);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

        // v0
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Reed, v0);

        // v1
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Reed, v1);

        // v2
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Reed, v2);

        // up
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Reed, up);

        return attributeDescriptions;
    }

    static VkBuffer reedVertexBuffer;
    static VkBuffer reedIndexBuffer;
	static uint32_t reedIndexCount;

    static VkDeviceMemory reedVertexBufferMemory;
    static VkDeviceMemory reedIndexBufferMemory;

    static void CreateBladeVertexIndexBuffer(Device* device, VkCommandPool commandPool,
        std::vector<ReedVertex> vertexBuffer, std::vector <uint32_t> indexBuffer);
    static void DestroyBladeVertexIndexBuffer(Device* device);
    static VkBuffer GetBladeVertexBuffer() { return reedVertexBuffer; }
    static VkBuffer GetBladeIndexBuffer() { return reedIndexBuffer; }
};



struct ReedsDrawIndirect {
    uint32_t    indexCount;
    uint32_t    instanceCount;
    uint32_t    firstIndex;
    int32_t     vertexOffset;
    uint32_t    firstInstance;
};

class Reeds : public Model
{
private:
    VkBuffer reedsBuffer;
    VkBuffer culledReedsBuffer;
    VkBuffer numReedsBuffer;

    VkDeviceMemory reedsBufferrMemory;
    VkDeviceMemory culledReedsBufferMemory;
    VkDeviceMemory numReedsBufferMemory;

public:
    Reeds(Device* device, VkCommandPool commandPool, float planeDim, glm::vec3 offset = glm::vec3(0));
    VkBuffer GetReedsBuffer() const;
    VkBuffer GetCulledReedsBuffer() const;
    VkBuffer GetNumReedsBuffer() const;
    uint32_t reedsCount = 0;
    ~Reeds();

};

