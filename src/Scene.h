#pragma once

#include <glm/glm.hpp>
#include <chrono>

#include "Model.h"
#include "Blades.h"
#include "Reeds.h"

using namespace std::chrono;

struct Time {
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
};

struct Theme
{
    glm::vec3 reedCol = glm::vec3(0.6, 0.64, 0.57);
    float pad0;
	glm::vec3 grassCol = glm::vec3(0.45, 0.64, 0.57) * 0.6f;
    float pad1;
    glm::vec3 sunCol = glm::vec3(0.8, 0.55, 0.5);
    float ambientScale = 1.0f;
    glm::vec3 skyCol = glm::vec3(0.81, 0.665, 0.45) * 1.2f;
    float ambientBlend = 0.5f;
};

class Scene {
private:
    Device* device;
    
    VkBuffer timeBuffer;
    VkDeviceMemory timeBufferMemory;
    
    void* mappedData;

    std::vector<Model*> models;
    std::vector<Blades*> blades;
    std::vector<Reeds*> reeds;

high_resolution_clock::time_point startTime = high_resolution_clock::now();

public:
    Time time;
	Theme theme;

    Scene() = delete;
    Scene(Device* device);
    ~Scene();

    const std::vector<Model*>& GetModels() const;
    const std::vector<Blades*>& GetBlades() const;
	const std::vector<Reeds*>& GetReeds() const;
    
    void AddModel(Model* model);
    void AddBlades(Blades* blades);
	void AddReeds(Reeds* reeds);

    VkBuffer GetTimeBuffer() const;

    void UpdateTime();
    void BeginTime();
};
