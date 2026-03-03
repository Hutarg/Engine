#pragma once

#include "../Core/Application.h"

#include "../Graphics/Window.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Pipeline.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

#define BLUEBERRY_MAJOR_VERSION 1
#define BLUEBERRY_MINOR_VERSION 0
#define BLUEBERRY_PATCH_VERSION 0

#define BLUEBERRY_MAX_FRAMES_IN_FLIGHT 3
#define BLUEBERRY_MAX_TEXTURE_COUNT 100000

namespace blueberry
{
	struct Application::Instance_T
	{
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};

	struct Application::PhysicalDevice_T
	{
		VkPhysicalDevice device;
		uint32_t presentQueueIndex;
		uint32_t graphicsQueueIndex;

		uint32_t maxAnisotropy;
		bool bindlessSupported;
	};

	struct Application::LogicalDevice_T
	{
		VkDevice device;
		VkQueue presentQueue;
		VkQueue graphicsQueue;
	};

	struct Application::Engine_T
	{
		VkCommandPool commandPool;
		TypeList<VkCommandBuffer> commandBuffers;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		TypeList<VkDescriptorSet> descriptorSets;

		VkDescriptorSetLayout bindlessDescriptorSetLayout;
		VkDescriptorPool bindlessDescriptorPool;
		VkDescriptorSet bindlessDescriptorSet;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		size_t stagingBufferSize;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		size_t vertexBufferSize;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		size_t indexBufferSize;

		TypeList<VkBuffer> transformBuffers;
		TypeList<VkDeviceMemory> transformBufferMemories;
		TypeList<size_t> transformBufferSizes;

		TypeList<VkBuffer> spriteBuffers;
		TypeList<VkDeviceMemory> spriteBufferMemories;
		TypeList<size_t> spriteBufferSizes;

		TypeList<VkBuffer> uniformBuffers;
		TypeList<VkDeviceMemory> uniformBufferMemories;
		TypeList<void*> uniformBufferMaps;

		TypeList<VkFence> inFlightFences;
	};

	struct Window::Window_T
	{
		GLFWwindow* window;

		VkSurfaceKHR surface;
		VkSurfaceCapabilitiesKHR capabilities;
		VkExtent2D extent;
		VkSwapchainKHR swapchain;
		TypeList<VkImage> images;
		TypeList<VkImageView> imageViews;
		TypeList<VkFramebuffer> framebuffers;

		TypeList<VkSemaphore> imageAvailableSemaphores;
		TypeList<VkSemaphore> renderFinishedSemaphores;

		bool isResized = false;
	};

	struct Window::WindowInfos_T
	{
		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;
		VkRenderPass renderPass;
	};

	struct Pipeline::Pipeline_T
	{
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
	};

	struct Shader::Shader_T
	{
		VkShaderModule shader;
		VkShaderStageFlagBits stage;
		const char* stageName;
	};

	struct Texture::Texture_T
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;

		bool update;

		// Animation :
		std::function<Vector4(double)> getUV;
	};

	struct alignas(16) Vertex
	{
		glm::vec3 pos;

		Vertex() : pos() {}
		Vertex(glm::vec3 pos) : pos(pos) {}
	};

	struct alignas(16) UBO
	{
		glm::mat4 proj;
		glm::mat4 view;

		UBO(glm::mat4 proj, glm::mat4 view) : proj(proj), view(view) {}
	};

	struct alignas(16) SSBO
	{
		glm::mat4 pos;
		glm::mat4 scale;
		glm::mat4 rotation;
		uint32_t viewportIndex;
	};
}