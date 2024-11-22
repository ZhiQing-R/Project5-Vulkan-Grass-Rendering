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

// ref: https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
glm::vec3 mod289(glm::vec3 x) {
    return x - glm::floor(x * (1.f / 289.f)) * 289.f;
}

glm::vec2 mod289(glm::vec2 x) {
    return x - glm::floor(x * (1.f / 289.f)) * 289.f;
}

glm::vec3 permute(glm::vec3 x) {
    return mod289(((x * 34.f) + 10.f) * x);
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

        currentBlade.v0 = glm::vec4(bladePosition, direction);

        // Bezier point and height (v1)
        float height = MIN_HEIGHT + (generateRandomFloat() * (MAX_HEIGHT - MIN_HEIGHT)) + 3.f * glm::exp(-0.5f * distToCenter);
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
        bladeIndexData.size() * sizeof(glm::vec2), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Blade::bladeIndexBuffer, Blade::bladeIndexBufferMemory);
}

void Blade::DestroyBladeVertexIndexBuffer(Device* device)
{
	vkDestroyBuffer(device->GetVkDevice(), bladeVertexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), bladeVertexBufferMemory, nullptr);
	vkDestroyBuffer(device->GetVkDevice(), bladeIndexBuffer, nullptr);
	vkFreeMemory(device->GetVkDevice(), bladeIndexBufferMemory, nullptr);
}
