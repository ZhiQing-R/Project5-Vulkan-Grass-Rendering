#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Scene.h"
#include "Camera.h"

class Renderer {
public:
    Renderer() = delete;
    Renderer(Device* device, SwapChain* swapChain, Scene* scene, Camera* camera);
    ~Renderer();

    void CreateCommandPools();

    void CreateRenderPass();
	void CreatePostProcessRenderPass();

    void CreateCameraDescriptorSetLayout();
    void CreateModelDescriptorSetLayout();
    void CreateTimeDescriptorSetLayout();
    void CreateComputeDescriptorSetLayout();
	void CreateColorDepthDescriptorSetLayout();
	void CreateNoiseMapDescriptorSetLayout();

    void CreateDescriptorPool();

    void CreateCameraDescriptorSet();
    void CreateModelDescriptorSets();
    void CreateGrassDescriptorSets();
    void CreateTimeDescriptorSet();
    void CreateComputeDescriptorSets();
    void CreateReedsComputeDescriptorSets();
	void CreateColorDepthDescriptorSet();
	void CreateNoiseMapDescriptorSet();

    void CreateGraphicsPipeline();
    void CreateGrassPipeline();
    void CreateComputePipeline();
    void CreateGrassInstancedPipeline();
    void CreateReedInstancedPipeline();
	void CreatePostProcessPipeline();

    void CreateFrameResources();
    void DestroyFrameResources();
    void RecreateFrameResources();

    void RecordCommandBuffers();
    void RecordComputeCommandBuffer();
	void RecordGrassCommandBuffer();
	void RecordPostProcessCommandBuffer();

    void Frame();

private:
    Device* device;
    VkDevice logicalDevice;
    SwapChain* swapChain;
    Scene* scene;
    Camera* camera;

    VkCommandPool graphicsCommandPool;
    VkCommandPool computeCommandPool;

    VkRenderPass renderPass;
	VkRenderPass postProcessRenderPass;

    VkDescriptorSetLayout cameraDescriptorSetLayout;
    VkDescriptorSetLayout modelDescriptorSetLayout;
    VkDescriptorSetLayout timeDescriptorSetLayout;
    VkDescriptorSetLayout bladesBufferDescriptorSetLayout;
    VkDescriptorSetLayout culledBladesBufferDescriptorSetLayout;
	VkDescriptorSetLayout numBladesDescriptorSetLayout;
	VkDescriptorSetLayout colorDepthDescriptorSetLayout;
	VkDescriptorSetLayout noiseMapDescriptorSetLayout;
    
    VkDescriptorPool descriptorPool;

    VkDescriptorSet cameraDescriptorSet;
    std::vector<VkDescriptorSet> modelDescriptorSets;
	std::vector<VkDescriptorSet> grassDescriptorSets;
    VkDescriptorSet timeDescriptorSet;
    std::vector<VkDescriptorSet> bladesBufferDescriptorSets;
    std::vector<VkDescriptorSet> culledBladesBufferDescriptorSets;
    std::vector<VkDescriptorSet> numBladesDescriptorSets;
    std::vector<VkDescriptorSet> reedsBufferDescriptorSets;
    std::vector<VkDescriptorSet> culledReedsBufferDescriptorSets;
    std::vector<VkDescriptorSet> numReedsDescriptorSets;
    VkDescriptorSet colorDepthDescriptorSet;
	VkDescriptorSet noiseMapDescriptorSet;

    VkPipelineLayout graphicsPipelineLayout;
    VkPipelineLayout grassPipelineLayout;
    VkPipelineLayout computePipelineLayout;
	VkPipelineLayout grassInstancedPipelineLayout;
	VkPipelineLayout reedInstancedPipelineLayout;
	VkPipelineLayout postProcessPipelineLayout;

    VkPipeline graphicsPipeline;
    VkPipeline grassPipeline;
    VkPipeline computePipeline;
	VkPipeline grassInstancedPipeline;
	VkPipeline reedInstancedPipeline;
	VkPipeline postProcessPipeline;

    std::vector<VkImageView> imageViews;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
	VkImage resultImage;
	VkDeviceMemory resultImageMemory;
	VkImageView resultImageView;

    VkImage noiseImage;
    VkDeviceMemory noiseImageMemory;
	VkImageView noiseImageView;
	VkSampler noiseSampler;

    VkSampler colorSampler;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkFramebuffer> resultImageFramebuffers;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> postCommandBuffers;
    VkCommandBuffer computeCommandBuffer;

};
