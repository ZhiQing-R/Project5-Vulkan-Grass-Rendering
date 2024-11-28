#include "Model.h"
#include "BufferUtils.h"
#include "Image.h"

Model::Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
  : device(device), vertices(vertices), indices(indices) {

    if (vertices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->vertices.data(), vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    }

    if (indices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->indices.data(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
    }

    modelBufferObject.modelMatrix = glm::mat4(1.0f);
    BufferUtils::CreateBufferFromData(device, commandPool, &modelBufferObject, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, modelBuffer, modelBufferMemory);
}

Model::~Model() {
    if (indices.size() > 0) {
        vkDestroyBuffer(device->GetVkDevice(), indexBuffer, nullptr);
        vkFreeMemory(device->GetVkDevice(), indexBufferMemory, nullptr);
    }

    if (vertices.size() > 0) {
        vkDestroyBuffer(device->GetVkDevice(), vertexBuffer, nullptr);
        vkFreeMemory(device->GetVkDevice(), vertexBufferMemory, nullptr);
    }

    vkDestroyBuffer(device->GetVkDevice(), modelBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), modelBufferMemory, nullptr);

    if (textureView != VK_NULL_HANDLE) {
        vkDestroyImageView(device->GetVkDevice(), textureView, nullptr);
    }

    if (textureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device->GetVkDevice(), textureSampler, nullptr);
    }
}

void Model::SetTexture(VkImage texture) {
    this->texture = texture;
    this->textureView = Image::CreateView(device, texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    // --- Specify all filters and transformations ---
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // Interpolation of texels that are magnified or minified
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // Addressing mode
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Anisotropic filtering
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;

    // Border color
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // Choose coordinate system for addressing texels --> [0, 1) here
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    // Comparison function used for filtering operations
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // Mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device->GetVkDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

const std::vector<Vertex>& Model::getVertices() const {
    return vertices;
}

VkBuffer Model::getVertexBuffer() const {
    return vertexBuffer;
}

const std::vector<uint32_t>& Model::getIndices() const {
    return indices;
}

VkBuffer Model::getIndexBuffer() const {
    return indexBuffer;
}

const ModelBufferObject& Model::getModelBufferObject() const {
    return modelBufferObject;
}

VkBuffer Model::GetModelBuffer() const {
    return modelBuffer;
}

VkImageView Model::GetTextureView() const {
    return textureView;
}

VkSampler Model::GetTextureSampler() const {
    return textureSampler;
}



float snoise(glm::vec2 v)
{
    const glm::vec4 C = glm::vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,  // -1.0 + 2.0 * C.x
        0.024390243902439); // 1.0 / 41.0
    // First corner
    glm::vec2 i = glm::floor(v + glm::dot(v, glm::vec2(C.y)));
    glm::vec2 x0 = v - i + glm::dot(i, glm::vec2(C.x));

    // Other corners
    glm::vec2 i1;
    i1 = (x0.x > x0.y) ? glm::vec2(1.0, 0.0) : glm::vec2(0.0, 1.0);
    glm::vec4 x12 = glm::vec4(x0.x, x0.y, x0.x, x0.y) + glm::vec4(C.x, C.x, C.z, C.z);
    x12.x -= i1.x;
    x12.y -= i1.y;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    glm::vec3 p = permute(permute(i.y + glm::vec3(0.0, i1.y, 1.0))
        + i.x + glm::vec3(0.0, i1.x, 1.0));

    glm::vec3 m = glm::max(0.5f - glm::vec3(glm::dot(x0, x0),
        glm::dot(glm::vec2(x12), glm::vec2(x12)), glm::dot(glm::vec2(x12.z, x12.w), glm::vec2(x12.z, x12.w))), 0.f);
    m = m * m;
    m = m * m;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    glm::vec3 x = 2.f * glm::fract(p * glm::vec3(C.w)) - 1.f;
    glm::vec3 h = glm::abs(x) - 0.5f;
    glm::vec3 ox = glm::floor(x + 0.5f);
    glm::vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);

    // Compute final noise value at P
    glm::vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    glm::vec2 tmp = glm::vec2(a0.y, a0.z) * glm::vec2(x12.x, x12.z) + glm::vec2(h.y, h.z) * glm::vec2(x12.y, x12.w);
    g.y = tmp.x;
    g.z = tmp.y;
    return 130.f * glm::dot(m, g);
}

float terrainHeight(glm::vec2 v)
{
    return snoise(v * 0.01f) * 10.f;
}

float generateRandomFloat() {
    return rand() / (float)RAND_MAX;
}

glm::vec2 hash22(glm::vec2 p)
{
    glm::vec3 p3 = glm::fract(glm::vec3(p.x, p.y, p.x) * glm::vec3(.1031, .1030, .0973));
    p3 += glm::dot(p3, glm::vec3(p3.y, p3.z, p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x, p3.x) + glm::vec2(p3.y, p3.z)) * glm::vec2(p3.z, p3.y));
}

glm::vec2 hash32(glm::vec3 p3)
{
    p3 = glm::fract(p3 * glm::vec3(.1031, .1030, .0973));
    p3 += glm::dot(p3, glm::vec3(p3.y, p3.z, p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x, p3.x) + glm::vec2(p3.y, p3.z)) * glm::vec2(p3.z, p3.y));
}