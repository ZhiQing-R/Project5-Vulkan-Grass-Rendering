#include "Reeds.h"
#include <vector>
#include "BufferUtils.h"
#include "glm/gtc/noise.hpp"

VkBuffer Reed::reedVertexBuffer = 0;
VkBuffer Reed::reedIndexBuffer = 0;
VkDeviceMemory Reed::reedVertexBufferMemory = 0;
VkDeviceMemory Reed::reedIndexBufferMemory = 0;
uint32_t Reed::reedIndexCount = 0;

#define UNIFORM_SPAWN 0

Reeds::Reeds(Device* device, VkCommandPool commandPool, float planeDim, glm::vec3 offset) : Model(device, commandPool, {}, {})
{
    std::vector<Reed> reeds;
    reeds.resize(NUM_REED * NUM_REED);
	float gridSize = planeDim / NUM_REED;

    for (int i = 0; i < NUM_REED; i++) {
        for (int j = 0; j < NUM_REED; j++) {
			glm::vec2 gridBase = -0.5f * glm::vec2(planeDim) + glm::vec2(i * gridSize, j * gridSize);
#if UNIFORM_SPAWN
            float spawnChance = 0.5f;
#else
            float spawnChance = (1.f - glm::perlin(0.03f * gridBase + 146.1413f) * 2.4f) * 0.3f;
#endif
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			if (r > spawnChance) {
				continue;
			}
            
            Reed currentBlade = Reed();

            glm::vec3 bladeUp = glm::normalize(glm::vec3(0.f, 1.0f, 0.f));

            // Generate positions and direction (v0)
            float x = generateRandomFloat() * gridSize + gridBase.x;
            float z = generateRandomFloat() * gridSize + gridBase.y;
            float y = 0.f;
            float direction = generateRandomFloat() * 0.2f * 3.14159265f + 1.3f * 3.14159265f;
            glm::vec3 bladePosition(x, y, z);
            bladePosition += offset;
            glm::vec2 bladeXZPosition(bladePosition.x, bladePosition.z);
            bladePosition.y = terrainHeight(bladeXZPosition);

            currentBlade.v0 = glm::vec4(bladePosition, direction);

            // Bezier point and height (v1)
            float height = REED_MIN_HEIGHT + (generateRandomFloat() * (REED_MAX_HEIGHT - REED_MIN_HEIGHT));
            currentBlade.v1 = glm::vec4(bladePosition + bladeUp * height, height);

            // Physical model guide and width (v2)
            float width = REED_MIN_WIDTH + (generateRandomFloat() * (REED_MAX_WIDTH - REED_MIN_WIDTH));
            currentBlade.v2 = glm::vec4(bladePosition + bladeUp * height, width);

            // Up vector and stiffness coefficient (up)
            float stiffness = REED_MIN_BEND + (generateRandomFloat() * (REED_MAX_BEND - REED_MIN_BEND));
            currentBlade.up = glm::vec4(bladeUp, stiffness);

            reeds[reedsCount++] = currentBlade;
        }
    }

    ReedsDrawIndirect indirectDraw;
    /*indirectDraw.vertexCount = NUM_BLADES;
    indirectDraw.instanceCount = 1;
    indirectDraw.firstVertex = 0;
    indirectDraw.firstInstance = 0;*/
    indirectDraw.indexCount = Reed::reedIndexCount;
    indirectDraw.instanceCount = 0;
    indirectDraw.firstIndex = 0;
    indirectDraw.vertexOffset = 0;
    indirectDraw.firstInstance = 0;

    BufferUtils::CreateBufferFromData(device, commandPool, reeds.data(), reeds.size() * sizeof(Reed), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reedsBuffer, reedsBufferrMemory);
    BufferUtils::CreateBuffer(device, reeds.size() * sizeof(Reed), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, culledReedsBuffer, culledReedsBufferMemory);
    BufferUtils::CreateBufferFromData(device, commandPool, &indirectDraw, sizeof(ReedsDrawIndirect), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, numReedsBuffer, numReedsBufferMemory);
}

VkBuffer Reeds::GetReedsBuffer() const
{
	return reedsBuffer;
}

VkBuffer Reeds::GetCulledReedsBuffer() const
{
	return culledReedsBuffer;
}

VkBuffer Reeds::GetNumReedsBuffer() const
{
	return numReedsBuffer;
}

Reeds::~Reeds()
{
	vkDestroyBuffer(device->GetVkDevice(), reedsBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), reedsBufferrMemory, nullptr);
	vkDestroyBuffer(device->GetVkDevice(), culledReedsBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), culledReedsBufferMemory, nullptr);
	vkDestroyBuffer(device->GetVkDevice(), numReedsBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), numReedsBufferMemory, nullptr);
}

void Reed::CreateBladeVertexIndexBuffer(Device* device, VkCommandPool commandPool, std::vector<ReedVertex> vertexBuffer, std::vector<uint32_t> indexBuffer)
{
	BufferUtils::CreateBufferFromData(device, commandPool, vertexBuffer.data(),
		vertexBuffer.size() * sizeof(ReedVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Reed::reedVertexBuffer, Reed::reedVertexBufferMemory);
	BufferUtils::CreateBufferFromData(device, commandPool, indexBuffer.data(),
		indexBuffer.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Reed::reedIndexBuffer, Reed::reedIndexBufferMemory);
	reedIndexCount = indexBuffer.size();
}

void Reed::DestroyBladeVertexIndexBuffer(Device* device)
{
	vkDestroyBuffer(device->GetVkDevice(), reedVertexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), reedVertexBufferMemory, nullptr);
	vkDestroyBuffer(device->GetVkDevice(), reedIndexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), reedIndexBufferMemory, nullptr);
}
