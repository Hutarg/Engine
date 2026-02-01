#pragma once

#include "../Core/Application.h"

#include "../Graphics/Window.h"
#include "../Graphics/Shader.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		VkCommandBuffer commandBuffer;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		int stagingBufferSize;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		int vertexBufferSize;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		int indexBufferSize;

		TypeList<VkBuffer> ssboBuffers;
		TypeList<VkDeviceMemory> ssboBufferMemories;
		TypeList<int> ssboBufferSizes;

		TypeList<VkBuffer> uniformBuffers;
		TypeList<VkDeviceMemory> uniformBufferMemories;
		TypeList<void*> uniformBufferMaps;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		TypeList<VkDescriptorSet> descriptorSets;
	};

	struct Window::Window_T
	{
		GLFWwindow* window;

		VkSurfaceKHR surface;
		VkExtent2D extent;
		VkSwapchainKHR swapchain;
		TypeList<VkImage> images;
		TypeList<VkImageView> imageViews;
		TypeList<VkFramebuffer> framebuffers;
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
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;
	};

	struct Shader::Shader_T
	{
		VkShaderModule shader;
		VkShaderStageFlagBits stage;
		const char* stageName;
	};

	struct alignas(16) Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 uv;

		Vertex() : pos(), normal(), uv() {}
		Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 uv) : pos(pos), normal(normal), uv(uv) {}
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