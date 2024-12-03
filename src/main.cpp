#include <vulkan/vulkan.h>
#include "Instance.h"
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Image.h"
#include <iostream>
#include "tiny_obj_loader.h"

Device* device;
SwapChain* swapChain;
Renderer* renderer;
Camera* camera;

namespace {
    void resizeCallback(GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) return;

        vkDeviceWaitIdle(device->GetVkDevice());
        swapChain->Recreate();
        renderer->RecreateFrameResources();
    }

    bool leftMouseDown = false;
    bool rightMouseDown = false;
    bool midMouseDown = false;
    double previousX = 0.0;
    double previousY = 0.0;

    void mouseDownCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                leftMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                leftMouseDown = false;
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                rightMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                rightMouseDown = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (action == GLFW_PRESS) {
                midMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                midMouseDown = false;
            }
        }
    }

    void mouseMoveCallback(GLFWwindow* window, double xPosition, double yPosition) {
        if (leftMouseDown) {
            double sensitivity = 0.5;
            float deltaX = static_cast<float>((previousX - xPosition) * sensitivity);
            float deltaY = static_cast<float>((previousY - yPosition) * sensitivity);

            camera->UpdateOrbit(deltaX, deltaY, 0.0f);

            previousX = xPosition;
            previousY = yPosition;
        } else if (rightMouseDown) {
            double deltaZ = static_cast<float>((previousY - yPosition) * 0.05);

            camera->UpdateOrbit(0.0f, 0.0f, deltaZ);

            previousY = yPosition;
        }
        else if (midMouseDown)
        {
            double sensitivity = 0.02;
            float deltaX = static_cast<float>((previousX - xPosition) * sensitivity);
            float deltaY = static_cast<float>((previousY - yPosition) * sensitivity);
			previousX = xPosition;
			previousY = yPosition;

            camera->UpdatePosition(deltaX, deltaY, 0.0f);
        }
    }

    void setTheme(Scene* scene)
    {
        scene->theme.reedCol = glm::vec3(0.6, 0.64, 0.57);
        scene->theme.grassCol = glm::vec3(0.45, 0.64, 0.57) * 0.6f;
        scene->theme.sunCol = glm::vec3(1.2, 0.75, 0.5);
        scene->theme.skyCol = glm::vec3(0.9, 0.665, 0.45) * 1.0f;
        scene->theme.ambientScale = 1.0f;
        scene->theme.ambientBlend = 0.7f;
    }

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			case GLFW_KEY_1:
            {
                // Luminescence of Eventide
				Scene* scene = renderer->GetScene();
				scene->theme.reedCol = glm::vec3(0.6, 0.64, 0.57);
                scene->theme.grassCol = glm::vec3(0.45, 0.64, 0.57) * 0.6f;
				scene->theme.sunCol = glm::vec3(1.2, 0.75, 0.5);
				scene->theme.skyCol = glm::vec3(0.9, 0.665, 0.45) * 1.0f;
                scene->theme.ambientScale = 1.0f;
                scene->theme.ambientBlend = 0.7f;
				renderer->reRecord = true;
				break;
            }
            case GLFW_KEY_2:
            {
                // Night Sky's Grace
				Scene* scene = renderer->GetScene();
				scene->theme.reedCol = glm::vec3(0.6, 0.64, 0.57);
                scene->theme.grassCol = glm::vec3(0.24, 0.45, 0.33) * 0.6f;
                scene->theme.reedCol = glm::vec3(0.6, 0.74, 0.77);
				scene->theme.sunCol = glm::vec3(0.4, 0.35, 0.5) * 1.2f;
				scene->theme.skyCol = glm::vec3(0.51, 0.665, 0.85) * 0.5f;
                scene->theme.ambientScale = 2.5f;
                scene->theme.ambientBlend = 0.7f;
                renderer->reRecord = true;
			    break;
            }
            case GLFW_KEY_3:
            {
                // Fresh Breezes of Dawn
                Scene* scene = renderer->GetScene();
                scene->theme.reedCol = glm::vec3(0.6, 0.64, 0.57);
                scene->theme.grassCol = glm::vec3(0.39, 0.55, 0.36) * 0.7f;
                scene->theme.sunCol = glm::vec3(0.9, 0.65, 0.5);
                scene->theme.skyCol = glm::vec3(0.41, 0.515, 0.55) * 1.8f;
                scene->theme.ambientScale = 1.0f;
                scene->theme.ambientBlend = 0.8f;
                renderer->reRecord = true;
                break;
            }
            case GLFW_KEY_4:
            {
                // The Fading Stories
                Scene* scene = renderer->GetScene();
                scene->theme.reedCol = glm::vec3(0.7, 0.84, 0.87) * 0.8f;
                scene->theme.grassCol = glm::vec3(0.31, 0.55, 0.37) * 0.8f;
                scene->theme.sunCol = glm::vec3(0.7, 0.65, 0.6);
                scene->theme.skyCol = glm::vec3(0.41, 0.565, 0.75) * 1.99f;
                scene->theme.ambientScale = 1.0f;
				scene->theme.ambientBlend = 0.9f;
                renderer->reRecord = true;
                break;
            }
            case GLFW_KEY_R:
            {
				renderer->renderReeds = !renderer->renderReeds;
                renderer->reRecord = true;
                break;
            }
            case GLFW_KEY_G:
            {
                renderer->renderGrass = !renderer->renderGrass;
                renderer->reRecord = true;
                break;
            }
            case GLFW_KEY_C:
            {
                Scene* scene = renderer->GetScene();
				scene->theme.renderCloud = (scene->theme.renderCloud + 1) % 2;
                renderer->reRecord = true;
                break;
            }
            default:
                break;
            }
		}
	}
}

int main() {
    static constexpr char* applicationName = "Vulkan Grass Rendering";
    InitializeWindow(1440, 1080, applicationName);

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Instance* instance = new Instance(applicationName, glfwExtensionCount, glfwExtensions);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance->GetVkInstance(), GetGLFWWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, surface);

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    device = instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, deviceFeatures);

    swapChain = device->CreateSwapChain(surface, 3);

    camera = new Camera(device, 640.f / 480.f);

    VkCommandPoolCreateInfo transferPoolInfo = {};
    transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Transfer];
    transferPoolInfo.flags = 0;

    VkCommandPool transferCommandPool;
    if (vkCreateCommandPool(device->GetVkDevice(), &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkImage grassImage;
    VkDeviceMemory grassImageMemory;
    Image::FromFile(device,
        transferCommandPool,
        "images/grass.jpg",
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        grassImage,
        grassImageMemory
    );

	std::vector<ReedVertex> reedVertices;
	std::vector<uint32_t> reedIndices;

    {
        std::string inputfile = "images/reed1.obj";
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

        if (!ret)
        {
            std::cout << err << std::endl;
            throw std::runtime_error("Failed to load reed object");
        }

        size_t s = 0;
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
				reedIndices.push_back(index_offset + v);
				ReedVertex vertex = {};
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0] / 2.f;
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1] / 2.f;
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2] / 4.f;
				//if (vy < 1.0) vz /= 2.f;
				vertex.pos = glm::vec4(vx, vy, vz, 1.0f);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					vertex.normal = glm::vec4(nx, ny, nz, 0.0f);
                }

				reedVertices.push_back(vertex);
            }
            index_offset += fv;
        }

		Reed::CreateBladeVertexIndexBuffer(device, transferCommandPool, reedVertices, reedIndices);
    }

    Scene* scene = new Scene(device);
	setTheme(scene);

    float planeDim = 15.f;
    float halfWidth = planeDim * 0.5f;

    glm::ivec2 terrainSize = { 20, 20 };

    for (int i = 0; i < terrainSize.x; ++i)
    {
        for (int j = 0; j < terrainSize.y; ++j)
        {
            glm::vec3 offset = { i * planeDim, 0, j * planeDim };
            Model* plane = new Model(device, transferCommandPool,
                {
                    { { -halfWidth + offset.x, 0.0f, halfWidth + offset.z }, { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
                    { { halfWidth + offset.x, 0.0f, halfWidth + offset.z }, { 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
                    { { halfWidth + offset.x, 0.0f, -halfWidth + offset.z }, { 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
                    { { -halfWidth + offset.x, 0.0f, -halfWidth + offset.z }, { 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
                },
                { 0, 1, 2, 2, 3, 0 }
            );
            plane->SetTexture(grassImage);
            scene->AddModel(plane);


			Blades* blades = new Blades(device, transferCommandPool, planeDim, offset);
            scene->AddBlades(blades);

        }
    }

    const int reedScale = 20;

    for (int i = 0; i < terrainSize.x / reedScale; ++i)
    {
        for (int j = 0; j < terrainSize.y / reedScale; ++j)
        {
            glm::vec3 offset = { i * planeDim * reedScale + halfWidth * (reedScale - 1), 0, j * planeDim * reedScale + halfWidth * (reedScale - 1) };

            Reeds* reeds = new Reeds(device, transferCommandPool, planeDim * reedScale, offset);
            scene->AddReeds(reeds);

        }
    }

    Blade::CreateBladeVertexIndexBuffer(device, transferCommandPool);

    vkDestroyCommandPool(device->GetVkDevice(), transferCommandPool, nullptr);


    renderer = new Renderer(device, swapChain, scene, camera);

    glfwSetWindowSizeCallback(GetGLFWWindow(), resizeCallback);
    glfwSetMouseButtonCallback(GetGLFWWindow(), mouseDownCallback);
    glfwSetCursorPosCallback(GetGLFWWindow(), mouseMoveCallback);
	glfwSetKeyCallback(GetGLFWWindow(), keyCallback);

    uint32_t frames = 0;
	scene->BeginTime();
    while (!ShouldQuit()) {
        glfwPollEvents();
        scene->UpdateTime();
        renderer->Frame();
        ++frames;
    }
	std::cout << "Average frame time: " << scene->time.totalTime / (float)frames << std::endl;

    vkDeviceWaitIdle(device->GetVkDevice());

    vkDestroyImage(device->GetVkDevice(), grassImage, nullptr);
    vkFreeMemory(device->GetVkDevice(), grassImageMemory, nullptr);
	Blade::DestroyBladeVertexIndexBuffer(device);
	Reed::DestroyBladeVertexIndexBuffer(device);

    delete scene;
    delete camera;
    delete renderer;
    delete swapChain;
    delete device;
	vkDestroySurfaceKHR(instance->GetVkInstance(), surface, nullptr);
    delete instance;
    DestroyWindow();

	getchar();
    return 0;
}
