#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

#include "Vertex.h"
#include "Device.h"

struct ModelBufferObject {
    glm::mat4 modelMatrix;
};

class Model {
protected:
    Device* device;

    std::vector<Vertex> vertices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    std::vector<uint32_t> indices;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer modelBuffer;
    VkDeviceMemory modelBufferMemory;

    ModelBufferObject modelBufferObject;

    VkImage texture = VK_NULL_HANDLE;
    VkImageView textureView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;

public:
    Model() = delete;
    Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    virtual ~Model();

    void SetTexture(VkImage texture);

    const std::vector<Vertex>& getVertices() const;

    VkBuffer getVertexBuffer() const;

    const std::vector<uint32_t>& getIndices() const;

    VkBuffer getIndexBuffer() const;

    const ModelBufferObject& getModelBufferObject() const;

    VkBuffer GetModelBuffer() const;
    VkImageView GetTextureView() const;
    VkSampler GetTextureSampler() const;
};


// ref: https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
inline glm::vec3 mod289(glm::vec3 x) {
    return x - glm::floor(x * (1.f / 289.f)) * 289.f;
}

inline glm::vec2 mod289(glm::vec2 x) {
    return x - glm::floor(x * (1.f / 289.f)) * 289.f;
}

inline glm::vec3 permute(glm::vec3 x) {
    return mod289(((x * 34.f) + 10.f) * x);
}

float snoise(glm::vec2 v);

float terrainHeight(glm::vec2 v);

float generateRandomFloat();

glm::vec2 hash22(glm::vec2 p);

glm::vec2 hash32(glm::vec3 p3);