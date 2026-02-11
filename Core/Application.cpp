#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>

#include "../Core/Entity.h"

#include "../Private/Structs.h"
#include "../Private/Functions.h"

#include "../Graphics/Window.h"

#define BLUEBERRY_MAJOR_VERSION 1
#define BLUEBERRY_MINOR_VERSION 0
#define BLUEBERRY_PATCH_VERSION 0

#ifndef BLUEBERRY_ENABLE_VALIDATION_LAYERS
#ifdef NDEBUG 
#define BLUEBERRY_ENABLE_VALIDATION_LAYERS false
#else
#define BLUEBERRY_ENABLE_VALIDATION_LAYERS true
#endif
#endif

const blueberry::TypeList<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const blueberry::TypeList<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace blueberry
{
	Application::Instance_T Application::instance_ = {};
	Application::PhysicalDevice_T Application::physicalDevice_ = {};
	Application::LogicalDevice_T Application::logicalDevice_ = {};
	Application::Engine_T Application::engine_ = {};

	bool Application::isRunning_ = false;
	uint32_t Application::maxFramesInFlight_ = 3;
	uint32_t Application::currentFrame_ = 0;

	// Crée l'instance et le debug messenger
	void Application::createInstance(const char* appName, int appMajorVersion, int appMinorVersion, int appPatchVersion)
	{
		// infos de l'app et du moteur
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = appName;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(appMajorVersion, appMinorVersion, appPatchVersion);
		applicationInfo.pEngineName = "Blueberry";
		applicationInfo.engineVersion = VK_MAKE_VERSION(BLUEBERRY_MAJOR_VERSION, BLUEBERRY_MINOR_VERSION, BLUEBERRY_PATCH_VERSION);
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		// infos pour l'instance
		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;

		// extensions nécessaire de glfw
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		TypeList<const char*> extensions = TypeList<const char*>(glfwExtensions, glfwExtensionCount);

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};

		if (BLUEBERRY_ENABLE_VALIDATION_LAYERS)
		{
			// Vérifie la disponibilité et ajoute les layers de validation
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					throw - 1;
				}
			}

			// Crée les infos du debug messenger maintenant pour que la création et destruction de l'instance puisse être debug
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
				| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
				| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = debugCallback;
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo;

			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

			extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
		}

		instanceCreateInfo.enabledExtensionCount = extensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_.instance) != VK_SUCCESS)
		{
			throw - 1;
		}

		// Crée le debug messenger
		if (BLUEBERRY_ENABLE_VALIDATION_LAYERS)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_.instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				if (func(instance_.instance, &debugMessengerCreateInfo, nullptr, &instance_.debugMessenger) != VK_SUCCESS)
				{
					throw - 1;
				}
			}
			else
			{
				throw - 1;
			}
		}
	}

	void Application::destroyInstance()
	{
	}

	/// <summary>
	/// Retourne les gpus qui peuvent être utilisé de manière que les "meilleurs" soit devant
	/// Ne prend pas directement le meilleur pcq'il faut checker si le gpu supporte les swap chains
	/// Qui ne peuvent être checker que avec la création d'une fenêtre.
	/// </summary>
	TypeList<Application::PhysicalDevice_T> Application::getPhysicalDevices()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_.instance, &deviceCount, nullptr);

		if (deviceCount == 0) throw - 1;

		TypeList<VkPhysicalDevice> physicalDevices = TypeList<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(instance_.instance, &deviceCount, physicalDevices.data());

		TypeList<uint32_t> scores = {};
		TypeList<PhysicalDevice_T> sortedPhysicalDevices = {};

		for (VkPhysicalDevice physicalDevice : physicalDevices)
		{
			// Teste si le composant est adapté (peut-être rajouter des tests)
			bool suitable = true;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

			if (!deviceFeatures.multiViewport) suitable = false;
			if (!deviceFeatures.geometryShader) suitable = false;

			// Check si le composant possède les queues nécessaire à l'affichage
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			TypeList<VkQueueFamilyProperties> queueFamiliesProperties = TypeList<VkQueueFamilyProperties>(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamiliesProperties.data());

			int presentQueueIndex = -1;
			int graphicsQueueIndex = -1;

			int i = 0;
			for (const VkQueueFamilyProperties& queueFamilyProperties : queueFamiliesProperties)
			{
				if (glfwGetPhysicalDevicePresentationSupport(instance_.instance, physicalDevice, i))
				{
					presentQueueIndex = i;
				}

				if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					graphicsQueueIndex = i;
				}

				i++;
			}

			if (graphicsQueueIndex == -1) suitable = false;
			if (presentQueueIndex == -1) suitable = false;

			// Check les extensions du gpu
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

			TypeList<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

			for (const char* deviceExtension : deviceExtensions)
			{
				bool found = false;

				for (const VkExtensionProperties& availableExtension : availableExtensions)
				{
					if (strcmp(availableExtension.extensionName, deviceExtension) == 0)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					suitable = false;
					break;
				}
			}

			if (!suitable) continue;

			// Attribution d'un score au composant
			int score = 0;

			score += deviceProperties.limits.maxImageDimension2D;
			score += deviceProperties.limits.maxImageDimension3D;

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

			// Range le gpu (il y a des meilleurs moyens mais c pas grave, il y a pas 10 000 gpu sur un ordi)
			PhysicalDevice_T physicalDevice_T = {};
			physicalDevice_T.device = physicalDevice;
			physicalDevice_T.presentQueueIndex = presentQueueIndex;
			physicalDevice_T.graphicsQueueIndex = graphicsQueueIndex;

			bool sorted = false;

			for (int i = 0; i < scores.size(); i++)
			{
				if (score > scores[i])
				{
					scores.add(score, i);
					sortedPhysicalDevices.add(physicalDevice_T, i);
					sorted = true;
					break;
				}
			}

			if (!sorted)
			{
				scores.add(score, scores.size());
				sortedPhysicalDevices.add(physicalDevice_T, sortedPhysicalDevices.size());
			}
		}

		if (sortedPhysicalDevices.size() == 0)
		{
			throw -1;
		}

		return sortedPhysicalDevices;
	}

	/// <summary>
	/// Crée le lien au gpu choisis par choosePhysicalDevice
	/// </summary>
	void Application::createLogicalDevice()
	{
		float priority = 1.0f;

		VkDeviceQueueCreateInfo presentQueueCreateInfo{};
		presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCreateInfo.queueFamilyIndex = physicalDevice_.presentQueueIndex;
		presentQueueCreateInfo.queueCount = 1;
		presentQueueCreateInfo.pQueuePriorities = &priority;

		VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = physicalDevice_.graphicsQueueIndex;
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = &priority;

		TypeList<VkDeviceQueueCreateInfo> queueCreateInfos = { presentQueueCreateInfo, graphicsQueueCreateInfo };

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.geometryShader = VK_TRUE;
		deviceFeatures.multiViewport = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (BLUEBERRY_ENABLE_VALIDATION_LAYERS)
		{
			deviceCreateInfo.enabledLayerCount = validationLayers.size();
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}

		if (vkCreateDevice(physicalDevice_.device, &deviceCreateInfo, nullptr, &logicalDevice_.device) != VK_SUCCESS)
		{
			throw - 1;
		}

		vkGetDeviceQueue(logicalDevice_.device, physicalDevice_.presentQueueIndex, 0, &logicalDevice_.presentQueue);
		vkGetDeviceQueue(logicalDevice_.device, physicalDevice_.graphicsQueueIndex, 0, &logicalDevice_.graphicsQueue);
	}

	void Application::destroyLogicalDevice()
	{
	}

	void Application::createEngine()
	{
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = physicalDevice_.graphicsQueueIndex;

		if (vkCreateCommandPool(logicalDevice_.device, &commandPoolInfo, nullptr, &engine_.commandPool) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = engine_.commandPool;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(logicalDevice_.device, &allocInfo, &engine_.commandBuffer) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding ssboLayoutBinding{};
		ssboLayoutBinding.binding = 1;
		ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssboLayoutBinding.descriptorCount = 1;
		ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		TypeList<VkDescriptorSetLayoutBinding> layoutBindings = { uboLayoutBinding, ssboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(logicalDevice_.device, &layoutInfo, nullptr, &engine_.descriptorSetLayout) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkDescriptorPoolSize uboPoolSize{};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = static_cast<uint32_t>(maxFramesInFlight_);

		VkDescriptorPoolSize ssboPoolSize{};
		ssboPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssboPoolSize.descriptorCount = static_cast<uint32_t>(maxFramesInFlight_);

		TypeList<VkDescriptorPoolSize> poolSizes = { uboPoolSize, ssboPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight_);

		if (vkCreateDescriptorPool(logicalDevice_.device, &descriptorPoolInfo, nullptr, &engine_.descriptorPool) != VK_SUCCESS)
		{
			throw - 1;
		}

		engine_.descriptorSets = TypeList<VkDescriptorSet>(maxFramesInFlight_);

		for (int i = 0; i < maxFramesInFlight_; i++)
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = engine_.descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &engine_.descriptorSetLayout;

			if (vkAllocateDescriptorSets(logicalDevice_.device, &allocInfo, &engine_.descriptorSets[i]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		engine_.stagingBuffer = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		engine_.stagingBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.stagingBuffer,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, 0);

		engine_.vertexBuffer = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		engine_.vertexBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.vertexBuffer,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.vertexBuffer, engine_.vertexBufferMemory, 0);

		engine_.indexBuffer = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		engine_.indexBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.indexBuffer,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.indexBuffer, engine_.indexBufferMemory, 0);

		engine_.uniformBuffers = TypeList<VkBuffer>(maxFramesInFlight_);
		engine_.uniformBufferMemories = TypeList<VkDeviceMemory>(maxFramesInFlight_);
		engine_.uniformBufferMaps = TypeList<void*>(maxFramesInFlight_);

		for (int i = 0; i < maxFramesInFlight_; i++)
		{
			engine_.uniformBuffers[i] = createBuffer(logicalDevice_.device, sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			engine_.uniformBufferMemories[i] = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.uniformBuffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkBindBufferMemory(logicalDevice_.device, engine_.uniformBuffers[i], engine_.uniformBufferMemories[i], 0);
			vkMapMemory(logicalDevice_.device, engine_.uniformBufferMemories[i], 0, sizeof(UBO), 0, &engine_.uniformBufferMaps[i]);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.uniformBuffers[i];
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = engine_.descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicalDevice_.device, 1, &descriptorWrite, 0, nullptr);
		}

		engine_.ssboBufferSizes = TypeList<size_t>(maxFramesInFlight_);
		engine_.ssboBuffers = TypeList<VkBuffer>(maxFramesInFlight_);
		engine_.ssboBufferMemories = TypeList<VkDeviceMemory>(maxFramesInFlight_);

		for (int i = 0; i < maxFramesInFlight_; i++)
		{
			engine_.ssboBufferSizes[i] = 1;
			engine_.ssboBuffers[i] = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			engine_.ssboBufferMemories[i] = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.ssboBuffers[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			vkBindBufferMemory(logicalDevice_.device, engine_.ssboBuffers[i], engine_.ssboBufferMemories[i], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.ssboBuffers[i];
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = engine_.descriptorSets[i];
			descriptorWrite.dstBinding = 1;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicalDevice_.device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void Application::destroyEngine()
	{
	}

	void Application::updateWindows()
	{
		for (Window::Window_T& window : Window::windows_)
		{
			glfwSwapBuffers(window.window);

			// modifie les composants vulkan de la fenêtre si sa taille a été modifiée

			if (window.isResized)
			{
				vkDeviceWaitIdle(Application::logicalDevice_.device);

				for (VkFramebuffer framebuffer : window.framebuffers) vkDestroyFramebuffer(Application::logicalDevice_.device, framebuffer, nullptr);
				for (VkImageView imageView : window.imageViews) vkDestroyImageView(Application::logicalDevice_.device, imageView, nullptr);
				// Les images sont détruites automatiquement avec la swapchain
				vkDestroySwapchainKHR(Application::logicalDevice_.device, window.swapchain, nullptr);

				if (window.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				{
					window.extent = window.capabilities.currentExtent;
				}
				else
				{
					int width, height;
					glfwGetWindowSize(window.window, &width, &height);

					window.extent = 
					{
						static_cast<uint32_t>(width),
						static_cast<uint32_t>(height)
					};

					window.extent.width = clamp(window.extent.width, window.capabilities.minImageExtent.width, window.capabilities.maxImageExtent.width);
					window.extent.height = clamp(window.extent.height, window.capabilities.minImageExtent.height, window.capabilities.maxImageExtent.height);
				}

				VkSwapchainCreateInfoKHR swapchainCreateInfo{};
				swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchainCreateInfo.surface = window.surface;
				swapchainCreateInfo.minImageCount = window.images.size();
				swapchainCreateInfo.imageFormat = Window::windowInfos_.format.format;
				swapchainCreateInfo.imageColorSpace = Window::windowInfos_.format.colorSpace;
				swapchainCreateInfo.imageExtent = { (unsigned int)window.extent.width, (unsigned int)window.extent.height };
				swapchainCreateInfo.imageArrayLayers = 1;
				swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

				if (Application::logicalDevice_.graphicsQueue != Application::logicalDevice_.presentQueue)
				{
					uint32_t queueFamilyIndices[] = { Application::physicalDevice_.graphicsQueueIndex, Application::physicalDevice_.presentQueueIndex };

					swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
					swapchainCreateInfo.queueFamilyIndexCount = 2;
					swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
				}

				swapchainCreateInfo.preTransform = window.capabilities.currentTransform;
				swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchainCreateInfo.presentMode = Window::windowInfos_.presentMode;
				swapchainCreateInfo.clipped = VK_TRUE;
				swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

				if (vkCreateSwapchainKHR(Application::logicalDevice_.device, &swapchainCreateInfo, nullptr, &window.swapchain) != VK_SUCCESS)
				{
					throw - 1;
				}

				// Récupère les images de la swapchain
				uint32_t imageCount = 0;
				vkGetSwapchainImagesKHR(Application::logicalDevice_.device, window.swapchain, &imageCount, nullptr);

				window.images = TypeList<VkImage>(imageCount);
				vkGetSwapchainImagesKHR(Application::logicalDevice_.device, window.swapchain, &imageCount, window.images.data());

				window.imageViews = TypeList<VkImageView>(imageCount);

				// Crée les images views
				VkImageViewCreateInfo imageViewCreateInfo{};
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				imageViewCreateInfo.subresourceRange.levelCount = 1;
				imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				imageViewCreateInfo.subresourceRange.layerCount = 1;

				for (int i = 0; i < window.images.size(); i++)
				{
					imageViewCreateInfo.image = window.images[i];
					imageViewCreateInfo.format = Window::windowInfos_.format.format;

					if (vkCreateImageView(Application::logicalDevice_.device, &imageViewCreateInfo, nullptr, &window.imageViews[i]) != VK_SUCCESS)
					{
						throw - 1;
					}
				}

				window.framebuffers = TypeList<VkFramebuffer>(window.imageViews.size());
				for (size_t i = 0; i < window.imageViews.size(); i++)
				{
					VkFramebufferCreateInfo framebufferInfo{};
					framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebufferInfo.renderPass = Window::windowInfos_.renderPass;
					framebufferInfo.attachmentCount = 1;
					framebufferInfo.pAttachments = &window.imageViews[i];
					framebufferInfo.width = window.extent.width;
					framebufferInfo.height = window.extent.height;
					framebufferInfo.layers = 1;

					if (vkCreateFramebuffer(Application::logicalDevice_.device, &framebufferInfo, nullptr, &window.framebuffers[i]) != VK_SUCCESS)
					{
						throw - 1;
					}
				}
			}

			window.isResized = false;
		}
	}

	void Application::drawSprites()
	{
		//if (Entity::components_.size() <= Entity::getComponentTypeID<Transform>()) return;

		TypeList<Vertex> vertices = {
				{ {-0.5f, -0.5f, 0.0f}, {1, 1, 0}, {0, 0, 0} },
				{ { 0.5f, -0.5f, 0.0f}, {0, 1, 1}, {1, 0, 0} },
				{ { 0.5f,  0.5f, 0.0f}, {0, 0, 1}, {1, 1, 0} },
				{ {-0.5f,  0.5f, 0.0f}, {1, 0, 1}, {0, 1, 0} }
		};

		TypeList<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

		int vertexBufferSize = vertices.size() * sizeof(Vertex);
		int indexBufferSize = indices.size() * sizeof(indices[0]);
		int ssboBufferSize = Entity::components_[Entity::getComponentTypeID<Transform>()].size() * sizeof(Transform);
		int stagingBufferSize = max(max(vertexBufferSize, indexBufferSize), ssboBufferSize);

		// Modification de la taille des buffers

		if (vertexBufferSize > engine_.vertexBufferSize)
		{
			engine_.vertexBufferSize = vertexBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.vertexBuffer, engine_.vertexBufferMemory, 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.vertexBuffer, engine_.vertexBufferMemory, 0);
		}

		if (indexBufferSize > engine_.indexBufferSize)
		{
			engine_.indexBufferSize = indexBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.indexBuffer, engine_.indexBufferMemory,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.indexBuffer, engine_.indexBufferMemory, 0);
		}

		if (ssboBufferSize > engine_.ssboBufferSizes[currentFrame_])
		{
			engine_.ssboBufferSizes[currentFrame_] = ssboBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.ssboBuffers[currentFrame_], engine_.ssboBufferMemories[currentFrame_],
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ssboBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.ssboBuffers[currentFrame_], engine_.ssboBufferMemories[currentFrame_], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.ssboBuffers[currentFrame_];
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = engine_.descriptorSets[currentFrame_];
			descriptorWrite.dstBinding = 1;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicalDevice_.device, 1, &descriptorWrite, 0, nullptr);
		}

		if (stagingBufferSize > engine_.stagingBufferSize)
		{
			engine_.stagingBufferSize = stagingBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, 0);
		}

		// Ecriture des données dans les buffers

		if (vertexBufferSize > 0)
		{
			void* data;
			vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, vertexBufferSize, 0, &data);
			memcpy(data, vertices.data(), vertexBufferSize);
			vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

			copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.vertexBuffer, vertexBufferSize);
		}

		if (indexBufferSize > 0)
		{
			void* data;
			vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, indexBufferSize, 0, &data);
			memcpy(data, indices.data(), indexBufferSize);
			vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

			copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.indexBuffer, indexBufferSize);
		}

		if (ssboBufferSize > 0)
		{
			void* data;
			vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, ssboBufferSize, 0, &data);
			memcpy(data, Entity::components_[Entity::getComponentTypeID<Transform>()].data()[0], ssboBufferSize);
			vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

			copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.ssboBuffers[currentFrame_], ssboBufferSize);
		}

		for (Pipeline::Pipeline_T pipeline : Pipeline::pipelines_)
		{
			for (Window::Window_T window : Window::windows_)
			{
				UBO ubo = { glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)), glm::perspective(glm::radians(60.0f), ((float)window.extent.width / (float)window.extent.height), 0.1f, 10.0f) };

				memcpy(engine_.uniformBufferMaps[currentFrame_], &ubo, sizeof(ubo));

				vkWaitForFences(logicalDevice_.device, 1, &pipeline.inFlightFence, VK_TRUE, UINT64_MAX);
				vkResetFences(logicalDevice_.device, 1, &pipeline.inFlightFence);

				uint32_t imageIndex;
				vkAcquireNextImageKHR(logicalDevice_.device, window.swapchain, UINT64_MAX, pipeline.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
				vkResetCommandBuffer(engine_.commandBuffer, 0);

				// Enregistre le command buffer pour le rendu des entités

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

				if (vkBeginCommandBuffer(engine_.commandBuffer, &beginInfo) != VK_SUCCESS)
				{
					throw - 1;
				}

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = Window::windowInfos_.renderPass;
				renderPassInfo.framebuffer = window.framebuffers[imageIndex];
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = window.extent;

				VkClearValue clearColor = { {{0.5f, 0.0f, 1.0f, 1.0f}} };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(engine_.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)window.extent.width;
				viewport.height = (float)window.extent.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = { (unsigned int)window.extent.width, (unsigned int)window.extent.height };

				vkCmdSetViewport(engine_.commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(engine_.commandBuffer, 0, 1, &scissor);

				vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

				VkDeviceSize offsets = 0;
				vkCmdBindVertexBuffers(engine_.commandBuffer, 0, 1, &engine_.vertexBuffer, &offsets);
				vkCmdBindIndexBuffer(engine_.commandBuffer, engine_.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

				vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &engine_.descriptorSets[currentFrame_], 0, nullptr);
				vkCmdDrawIndexed(engine_.commandBuffer, static_cast<uint32_t>(indices.size()), Entity::components_[Entity::getComponentTypeID<Transform>()].size(), 0, 0, 0);

				vkCmdEndRenderPass(engine_.commandBuffer);

				if (vkEndCommandBuffer(engine_.commandBuffer) != VK_SUCCESS)
				{
					throw - 1;
				}

				// Soumet le command buffer

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { pipeline.imageAvailableSemaphore };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &engine_.commandBuffer;

				VkSemaphore signalSemaphores[] = { pipeline.renderFinishedSemaphore };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;

				if (vkQueueSubmit(logicalDevice_.graphicsQueue, 1, &submitInfo, pipeline.inFlightFence) != VK_SUCCESS)
				{
					throw - 1;
				}

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapChains[] = { window.swapchain };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;
				presentInfo.pImageIndices = &imageIndex;

				vkQueuePresentKHR(logicalDevice_.presentQueue, &presentInfo);
			}
		}

		currentFrame_ = (currentFrame_ + 1) % maxFramesInFlight_;
	}

	void Application::updateScripts(float dt)
	{
		for (uint32_t scripts : Entity::scripts_)
		{
			for (void* script : Entity::components_[scripts])
			{
				if (script == nullptr) continue;

				((Script*)script)->update(dt);
			}
		}
	}

	void Application::init()
	{
		init("", 1, 0, 0);
	}

	void Application::init(const char* appName, int appMajorVersion, int appMinorVersion, int appPatchVersion)
    {
        if (!glfwInit()) return;
		createInstance(appName, appMajorVersion, appMinorVersion, appPatchVersion);

		physicalDevice_.device = VK_NULL_HANDLE;
    }

    void Application::terminate()
    {
		for (Window::Window_T window : Window::windows_)
		{
			glfwDestroyWindow(window.window);
		}

		if (physicalDevice_.device == VK_NULL_HANDLE)
		{
			destroyLogicalDevice();
		}

		destroyInstance();
        glfwTerminate();
    }

    void Application::run()
    {
        if (isRunning_) return;

        isRunning_ = true; 
		std::chrono::steady_clock::time_point lastTime = std::chrono::high_resolution_clock::now();

        while (isRunning_)
        {
			updateWindows();
			drawSprites();

			// Calcul du dt
			std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
			double dt = ((std::chrono::duration<double>)(currentTime - lastTime)).count();
			lastTime = currentTime;

			updateScripts(dt);

            glfwPollEvents();
        }
    }

    void Application::stop()
    {
        isRunning_ = true;
    }
}