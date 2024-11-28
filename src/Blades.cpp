#include <vector>
#include "Blades.h"
#include "BufferUtils.h"

static std::array<glm::vec2, 15> bladeVertexData =
{
    glm::vec2(0.f, 0.f),
    glm::vec2(1.f, 0.f),
    glm::vec2(0.f, 0.2f),
    glm::vec2(1.f, 0.2f),
    glm::vec2(0.f, 0.4f),
    glm::vec2(1.f, 0.4f),
    glm::vec2(0.f, 0.55f),
    glm::vec2(1.f, 0.55f),
    glm::vec2(0.f, 0.7f),
    glm::vec2(1.f, 0.7f),
    glm::vec2(0.f, 0.8f),
    glm::vec2(1.f, 0.8f),
    glm::vec2(0.f, 0.9f),
    glm::vec2(1.f, 0.9f),
    glm::vec2(0.5f, 1.f),
};

static std::array<uint32_t, 39> bladeIndexData =
{
    0, 1, 2,
    1, 3, 2,
    2, 3, 4,
    3, 5, 4,
    4, 5, 6,
    5, 7, 6,
    6, 7, 8,
    7, 9, 8,
    8, 9, 10,
    9, 11, 10,
    10, 11, 12,
    11, 13, 12,
    12, 13, 14
};

glm::vec4 getNearestClumpGrid(glm::vec2 position) {
	glm::ivec2 clumpGridID = glm::floor(position / ClumpGridSize);
	float minDist = 1000000.0f;
    glm::vec4 out;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
			glm::vec2 currGrid = clumpGridID + glm::ivec2(i, j);
            glm::vec2 gridOffset = hash22(currGrid) * ClumpGridSize;
			glm::vec2 gridCenter = currGrid * ClumpGridSize + gridOffset;
			float dist = glm::distance(position, gridCenter);
            if (dist < minDist) {
                minDist = dist;
                out = glm::vec4(currGrid, gridCenter);
            }
        }
    }
	return out;
}

#define USE_CLUMP 1


Blades::Blades(Device* device, VkCommandPool commandPool, float planeDim, glm::vec3 offset) : Model(device, commandPool, {}, {}) {
    std::vector<Blade> blades;
    blades.reserve(NUM_BLADES);

    for (int i = 0; i < NUM_BLADES; i++) {
        Blade currentBlade = Blade();

        glm::vec3 bladeUp(0.0f, 1.0f, 0.0f);

        // Generate positions and direction (v0)
        float x = (generateRandomFloat() - 0.5f) * planeDim;
        float y = 0.0f;
        float z = (generateRandomFloat() - 0.5f) * planeDim;
        float direction = generateRandomFloat() * 2.f * 3.14159265f;
        glm::vec3 bladePosition(x, y, z);
        bladePosition += offset;
		glm::vec2 bladeXZPosition(bladePosition.x, bladePosition.z);
		glm::vec4 clumpData = getNearestClumpGrid(bladeXZPosition);
		float distToCenter = glm::distance(bladeXZPosition, glm::vec2(clumpData.z, clumpData.w));

#if USE_CLUMP
        // shift to clump center a little bit
		bladeXZPosition = glm::mix(bladeXZPosition, glm::vec2(clumpData.z, clumpData.w), 0.01f);
		bladePosition.x = bladeXZPosition.x;
		bladePosition.z = bladeXZPosition.y;
		bladePosition.y += terrainHeight(bladeXZPosition);

        // face to the same direction
		float clumpDir = hash32(glm::vec3(clumpData.x, clumpData.y, 0.6f)).x * 2.f * 3.14159265f;
        direction = glm::mix(direction, clumpDir, 0.6f);

        // face off to center
		float offCenterDir = std::atan2(bladePosition.z - clumpData.w, bladePosition.x - clumpData.z) + 0.5f * 3.14159265f;
		direction = glm::mix(direction, offCenterDir, 0.4f);
#endif
        currentBlade.v0 = glm::vec4(bladePosition, direction);

        // Bezier point and height (v1)
        float height = MIN_HEIGHT + (generateRandomFloat() * (MAX_HEIGHT - MIN_HEIGHT)) + 6.f * glm::exp(-0.3f * distToCenter);
        currentBlade.v1 = glm::vec4(bladePosition + bladeUp * height, height);

        // Physical model guide and width (v2)
        float width = MIN_WIDTH + (generateRandomFloat() * (MAX_WIDTH - MIN_WIDTH));
        currentBlade.v2 = glm::vec4(bladePosition + bladeUp * height, width);

        // Up vector and stiffness coefficient (up)
        float stiffness = MIN_BEND + (generateRandomFloat() * (MAX_BEND - MIN_BEND));
        currentBlade.up = glm::vec4(bladeUp, stiffness);

        blades.push_back(currentBlade);
    }

    BladeDrawIndirect indirectDraw;
    /*indirectDraw.vertexCount = NUM_BLADES;
    indirectDraw.instanceCount = 1;
    indirectDraw.firstVertex = 0;
    indirectDraw.firstInstance = 0;*/
	indirectDraw.indexCount = bladeIndexData.size();
	indirectDraw.instanceCount = NUM_BLADES;
	indirectDraw.firstIndex = 0;
	indirectDraw.vertexOffset = 0;
	indirectDraw.firstInstance = 0;

    BufferUtils::CreateBufferFromData(device, commandPool, blades.data(), NUM_BLADES * sizeof(Blade), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, bladesBuffer, bladesBufferMemory);
    BufferUtils::CreateBuffer(device, NUM_BLADES * sizeof(Blade), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, culledBladesBuffer, culledBladesBufferMemory);
    BufferUtils::CreateBufferFromData(device, commandPool, &indirectDraw, sizeof(BladeDrawIndirect), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, numBladesBuffer, numBladesBufferMemory);
}

VkBuffer Blades::GetBladesBuffer() const {
    return bladesBuffer;
}

VkBuffer Blades::GetCulledBladesBuffer() const {
    return culledBladesBuffer;
}

VkBuffer Blades::GetNumBladesBuffer() const {
    return numBladesBuffer;
}

Blades::~Blades() {
    vkDestroyBuffer(device->GetVkDevice(), bladesBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), bladesBufferMemory, nullptr);
    vkDestroyBuffer(device->GetVkDevice(), culledBladesBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), culledBladesBufferMemory, nullptr);
    vkDestroyBuffer(device->GetVkDevice(), numBladesBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), numBladesBufferMemory, nullptr);
}

VkBuffer Blade::bladeVertexBuffer = 0;
VkBuffer Blade::bladeIndexBuffer = 0;
VkDeviceMemory Blade::bladeVertexBufferMemory = 0;
VkDeviceMemory Blade::bladeIndexBufferMemory = 0;

void Blade::CreateBladeVertexIndexBuffer(Device* device, VkCommandPool commandPool)
{
    BufferUtils::CreateBufferFromData(device, commandPool, bladeVertexData.data(),
        bladeVertexData.size() * sizeof(glm::vec2), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Blade::bladeVertexBuffer, Blade::bladeVertexBufferMemory);
    BufferUtils::CreateBufferFromData(device, commandPool, bladeIndexData.data(),
        bladeIndexData.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Blade::bladeIndexBuffer, Blade::bladeIndexBufferMemory);
}

void Blade::DestroyBladeVertexIndexBuffer(Device* device)
{
	vkDestroyBuffer(device->GetVkDevice(), bladeVertexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), bladeVertexBufferMemory, nullptr);
	vkDestroyBuffer(device->GetVkDevice(), bladeIndexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), bladeIndexBufferMemory, nullptr);
}
