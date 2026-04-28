#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>

#include "Entity.h"

#include "../Private/Structs.h"
#include "../Private/Functions.h"

#include "../Graphics/Window.h"
#include "../Graphics/Pipeline.h"

#include "../Utils/TypeList.h"

#ifndef BLUEBERRY_ENABLE_VALIDATION_LAYERS
#ifdef NDEBUG 
#define BLUEBERRY_ENABLE_VALIDATION_LAYERS false
#else
#define BLUEBERRY_ENABLE_VALIDATION_LAYERS true
#endif
#endif

const blueberry::TypeList<const char*> validationLayers = 
{
	"VK_LAYER_KHRONOS_validation"
};

const blueberry::TypeList<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace blueberry
{
	Application::Instance_T Application::instance_ = {};
	Application::PhysicalDevice_T Application::physicalDevice_ = {};
	Application::LogicalDevice_T Application::logicalDevice_ = {};
	Application::Engine_T Application::engine_ = {};

	const char* Application::defaultFolder_ = "";

	bool Application::isRunning_ = false;
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

			VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
			indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
			indexingFeatures.pNext = nullptr;

			VkPhysicalDeviceFeatures2 deviceFeatures2{};
			deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			deviceFeatures2.pNext = &indexingFeatures;

			vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

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

			bool bindlessSupported =	indexingFeatures.shaderSampledImageArrayNonUniformIndexing && 
										indexingFeatures.descriptorBindingSampledImageUpdateAfterBind &&
										indexingFeatures.shaderUniformBufferArrayNonUniformIndexing &&
										indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind &&
										indexingFeatures.shaderStorageBufferArrayNonUniformIndexing &&
										indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind;

			if (bindlessSupported) score += 1000;

			// Range le gpu (il y a des meilleurs moyens mais c très pas grave, il y a pas 10 000 gpu sur un ordi)
			PhysicalDevice_T physicalDevice_T = {};
			physicalDevice_T.device = physicalDevice;
			physicalDevice_T.presentQueueIndex = presentQueueIndex;
			physicalDevice_T.graphicsQueueIndex = graphicsQueueIndex;
			physicalDevice_T.bindlessSupported = bindlessSupported;
			physicalDevice_T.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

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

		if (physicalDevice_.bindlessSupported)
		{
			VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
			indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
			indexingFeatures.pNext = nullptr;

			VkPhysicalDeviceFeatures2 deviceFeatures2{};
			deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			deviceFeatures2.pNext = &indexingFeatures;

			vkGetPhysicalDeviceFeatures2(physicalDevice_.device, &deviceFeatures2);

			deviceFeatures2.features.geometryShader = VK_TRUE;
			deviceFeatures2.features.multiViewport = VK_TRUE;
			
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &deviceFeatures2;
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

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding transformLayoutBinding{};
		transformLayoutBinding.binding = 1;
		transformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		transformLayoutBinding.descriptorCount = 1;
		transformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding spriteLayoutBinding{};
		spriteLayoutBinding.binding = 2;
		spriteLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		spriteLayoutBinding.descriptorCount = 1;
		spriteLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		TypeList<VkDescriptorSetLayoutBinding> layoutBindings = { uboLayoutBinding, transformLayoutBinding, spriteLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(logicalDevice_.device, &layoutInfo, nullptr, &engine_.descriptorSetLayout) != VK_SUCCESS)
		{
			throw - 1;
		}

		if (physicalDevice_.bindlessSupported)
		{
			VkDescriptorSetLayoutBinding textureLayoutBinding{};
			textureLayoutBinding.binding = 0;
			textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureLayoutBinding.descriptorCount = BLUEBERRY_MAX_TEXTURE_COUNT;
			textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorBindingFlagsEXT bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
				VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
				VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

			VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flagsInfo = {};
			flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
			flagsInfo.pBindingFlags = &bindingFlags;
			flagsInfo.bindingCount = 1;

			VkDescriptorSetLayoutCreateInfo bindlessLayoutInfo = {};
			bindlessLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			bindlessLayoutInfo.bindingCount = 1;
			bindlessLayoutInfo.pBindings = &textureLayoutBinding;
			bindlessLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
			bindlessLayoutInfo.pNext = &flagsInfo;

			if (vkCreateDescriptorSetLayout(logicalDevice_.device, &bindlessLayoutInfo, nullptr, &engine_.bindlessDescriptorSetLayout) != VK_SUCCESS)
			{
				throw - 1;
			}
		}
		else
		{
			// faire le cas contraire
		}

		VkDescriptorPoolSize uboPoolSize{};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = static_cast<uint32_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolSize transformPoolSize{};
		transformPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		transformPoolSize.descriptorCount = static_cast<uint32_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolSize spritePoolSize{};
		spritePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		spritePoolSize.descriptorCount = static_cast<uint32_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		TypeList<VkDescriptorPoolSize> poolSizes = { uboPoolSize, transformPoolSize, spritePoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = static_cast<uint32_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(logicalDevice_.device, &descriptorPoolInfo, nullptr, &engine_.descriptorPool) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkDescriptorPoolSize texturePoolSize{};
		texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturePoolSize.descriptorCount = BLUEBERRY_MAX_TEXTURE_COUNT;

		VkDescriptorPoolCreateInfo bindlessDescriptorPoolInfo{};
		bindlessDescriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		bindlessDescriptorPoolInfo.poolSizeCount = 1;
		bindlessDescriptorPoolInfo.pPoolSizes = &texturePoolSize;
		bindlessDescriptorPoolInfo.maxSets = 1;
		bindlessDescriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		if (vkCreateDescriptorPool(logicalDevice_.device, &bindlessDescriptorPoolInfo, nullptr, &engine_.bindlessDescriptorPool) != VK_SUCCESS)
		{
			throw - 1;
		}

		TypeList<VkDescriptorSetLayout> descriptorSetLayouts = { engine_.descriptorSetLayout, BLUEBERRY_MAX_FRAMES_IN_FLIGHT };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = engine_.descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		engine_.descriptorSets = TypeList<VkDescriptorSet>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(logicalDevice_.device, &descriptorSetAllocateInfo, engine_.descriptorSets.data()) != VK_SUCCESS)
		{
			throw - 1;
		}

		TypeList<VkDescriptorSetLayout> bindlessDescriptorSetLayouts = { engine_.bindlessDescriptorSetLayout, BLUEBERRY_MAX_FRAMES_IN_FLIGHT };

		uint32_t counts = 100000;

		VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
		countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		countInfo.descriptorSetCount = 1;
		countInfo.pDescriptorCounts = &counts;

		VkDescriptorSetAllocateInfo bindlessDescriptorSetAllocateInfo{};
		bindlessDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		bindlessDescriptorSetAllocateInfo.descriptorPool = engine_.bindlessDescriptorPool;
		bindlessDescriptorSetAllocateInfo.descriptorSetCount = 1;
		bindlessDescriptorSetAllocateInfo.pSetLayouts = bindlessDescriptorSetLayouts.data();
		bindlessDescriptorSetAllocateInfo.pNext = &countInfo;

		if (vkAllocateDescriptorSets(logicalDevice_.device, &bindlessDescriptorSetAllocateInfo, &engine_.bindlessDescriptorSet) != VK_SUCCESS)
		{
			throw - 1;
		}

		TypeList<Vertex> vertices = {
			{ {-0.5f, -0.5f, 0.0f} },
			{ { 0.5f, -0.5f, 0.0f} },
			{ { 0.5f,  0.5f, 0.0f} },
			{ {-0.5f,  0.5f, 0.0f} }
		};

		TypeList<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

		size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		size_t indexBufferSize = indices.size() * sizeof(indices[0]);

		engine_.stagingBufferSize = max(vertexBufferSize, indexBufferSize);

		engine_.stagingBuffer = createBuffer(logicalDevice_.device, engine_.stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		engine_.stagingBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.stagingBuffer,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, 0);

		engine_.vertexBuffer = createBuffer(logicalDevice_.device, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		engine_.vertexBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.vertexBuffer,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.vertexBuffer, engine_.vertexBufferMemory, 0);

		engine_.indexBuffer = createBuffer(logicalDevice_.device, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		engine_.indexBufferMemory = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.indexBuffer,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkBindBufferMemory(logicalDevice_.device, engine_.indexBuffer, engine_.indexBufferMemory, 0);

		void* data;
		vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, vertexBufferSize, 0, &data);
		memcpy(data, vertices.data(), vertexBufferSize);
		vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

		copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.vertexBuffer, vertexBufferSize);

		vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, indices.data(), indexBufferSize);
		vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

		copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.indexBuffer, indexBufferSize);

		engine_.uniformBuffers = TypeList<VkBuffer>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.uniformBufferMemories = TypeList<VkDeviceMemory>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.uniformBufferMaps = TypeList<void*>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < BLUEBERRY_MAX_FRAMES_IN_FLIGHT; i++)
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

		engine_.transformBufferSizes = TypeList<size_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.transformBuffers = TypeList<VkBuffer>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.transformBufferMemories = TypeList<VkDeviceMemory>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < BLUEBERRY_MAX_FRAMES_IN_FLIGHT; i++)
		{
			engine_.transformBufferSizes[i] = 1;
			engine_.transformBuffers[i] = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			engine_.transformBufferMemories[i] = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.transformBuffers[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			vkBindBufferMemory(logicalDevice_.device, engine_.transformBuffers[i], engine_.transformBufferMemories[i], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.transformBuffers[i];
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

		engine_.spriteBufferSizes = TypeList<size_t>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.spriteBuffers = TypeList<VkBuffer>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		engine_.spriteBufferMemories = TypeList<VkDeviceMemory>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < BLUEBERRY_MAX_FRAMES_IN_FLIGHT; i++)
		{
			engine_.spriteBufferSizes[i] = 1;
			engine_.spriteBuffers[i] = createBuffer(logicalDevice_.device, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			engine_.spriteBufferMemories[i] = createDeviceMemory(physicalDevice_.device, logicalDevice_.device, engine_.spriteBuffers[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			vkBindBufferMemory(logicalDevice_.device, engine_.spriteBuffers[i], engine_.spriteBufferMemories[i], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.spriteBuffers[i];
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

		engine_.inFlightFences = TypeList<VkFence>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < BLUEBERRY_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			if (vkCreateFence(Application::logicalDevice_.device, &fenceInfo, nullptr, &engine_.inFlightFences[i]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		// Rajouter des valeurs par défaut directement intégrés dans le moteur
		engine_.defaultTexture = Texture(File((defaultFolder_ + String("/default.png")).cStr()));
		Shader defaultUiVertexShader = Shader((defaultFolder_ + String("/default.vert")).cStr(), ShaderType::VERTEX);
		Shader defaultUiFragmentShader = Shader((defaultFolder_ + String("/default.frag")).cStr(), ShaderType::FRAGMENT);

		engine_.defaultUiPipeline = Pipeline({ defaultUiVertexShader, defaultUiFragmentShader });

		defaultUiFragmentShader.destroy();
		defaultUiVertexShader.destroy();
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
				// Les images sont détruites automatiquement avec la swapchain donc pas de for image
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

	void Application::draw()
	{
		vkWaitForFences(logicalDevice_.device, 1, &engine_.inFlightFences[currentFrame_], VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice_.device, 1, &engine_.inFlightFences[currentFrame_]);

		// Update les textures

		for (int i = 0; i < Texture::textures_.size(); i++)
		{
			if (!Texture::textures_[i].update) continue;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.sampler = Texture::textures_[i].sampler;
			imageInfo.imageView = Texture::textures_[i].imageView;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = engine_.bindlessDescriptorSet;
			write.dstBinding = 0;
			write.dstArrayElement = i;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(Application::logicalDevice_.device, 1, &write, 0, nullptr);
		}

		size_t transformBufferSize = Entity::getComponentPool<Transform>().components.size() * sizeof(Transform);
		size_t spriteBufferSize = Entity::getComponentPool<Sprite>().components.size() * sizeof(Sprite);
		size_t stagingBufferSize = max(transformBufferSize, spriteBufferSize);

		// Modification de la taille des buffers si nécessaire

		if (stagingBufferSize > engine_.stagingBufferSize)
		{
			engine_.stagingBufferSize = stagingBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.stagingBuffer, engine_.stagingBufferMemory, 0);
		}

		if (transformBufferSize > engine_.transformBufferSizes[currentFrame_])
		{
			engine_.transformBufferSizes[currentFrame_] = transformBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.transformBuffers[currentFrame_], engine_.transformBufferMemories[currentFrame_],
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.transformBuffers[currentFrame_], engine_.transformBufferMemories[currentFrame_], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.transformBuffers[currentFrame_];
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

		if (spriteBufferSize > engine_.spriteBufferSizes[currentFrame_])
		{
			engine_.spriteBufferSizes[currentFrame_] = spriteBufferSize;

			recreateBuffer(physicalDevice_.device, logicalDevice_.device, engine_.spriteBuffers[currentFrame_], engine_.spriteBufferMemories[currentFrame_],
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, spriteBufferSize);

			vkBindBufferMemory(logicalDevice_.device, engine_.spriteBuffers[currentFrame_], engine_.spriteBufferMemories[currentFrame_], 0);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.buffer = engine_.spriteBuffers[currentFrame_];
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = engine_.descriptorSets[currentFrame_];
			descriptorWrite.dstBinding = 2;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicalDevice_.device, 1, &descriptorWrite, 0, nullptr);
		}

		// Ecriture des données dans les buffers

		if (transformBufferSize > 0)
		{
			void* data;
			vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, transformBufferSize, 0, &data);
			memcpy(data, Entity::getComponentPool<Transform>().components.data(), transformBufferSize);
			vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

			copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.transformBuffers[currentFrame_], transformBufferSize);
		}

		// Envoie toutes les données des sprites au shader qui va déterminer lui même si il render les sprites ou pas

		if (spriteBufferSize > 0)
		{
			void* data;
			vkMapMemory(logicalDevice_.device, engine_.stagingBufferMemory, 0, spriteBufferSize, 0, &data);
			memcpy(data, Entity::getComponentPool<Sprite>().components.data(), spriteBufferSize);
			vkUnmapMemory(logicalDevice_.device, engine_.stagingBufferMemory);

			copyBuffer(logicalDevice_.device, logicalDevice_.graphicsQueue, engine_.commandPool, engine_.stagingBuffer, engine_.spriteBuffers[currentFrame_], spriteBufferSize);
		}

		if (engine_.commandBuffers.size() < Window::windows_.size() * (Pipeline::pipelines_.size() + 1)) // augmente le nombre de command buffer si nécessaire afin de submit les draw call en même temps
		{
			int commandBufferSize = engine_.commandBuffers.size();
			engine_.commandBuffers.resize(Window::windows_.size()* (Pipeline::pipelines_.size() + 1));

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = Application::engine_.commandPool;
			allocInfo.commandBufferCount = Window::windows_.size() * (Pipeline::pipelines_.size() + 1) - commandBufferSize;

			if (vkAllocateCommandBuffers(Application::logicalDevice_.device, &allocInfo, &engine_.commandBuffers[commandBufferSize]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		TypeList<uint32_t> imageIndices = TypeList<uint32_t>(Window::windows_.size());
		TypeList<VkSwapchainKHR> swapchains = TypeList<VkSwapchainKHR>(Window::windows_.size());
		TypeList<VkPipelineStageFlags> waitStages = TypeList<VkPipelineStageFlags>(Window::windows_.size());

		TypeList<VkSemaphore> imageAvailableSemaphores = TypeList<VkSemaphore>(Window::windows_.size());
		TypeList<VkSemaphore> renderFinishedSemaphores = TypeList<VkSemaphore>(Window::windows_.size());

		for (int i = 0; i < Window::windows_.size(); i++)
		{
			Window::Window_T window = Window::windows_[i];

			uint32_t imageIndex;
			vkAcquireNextImageKHR(logicalDevice_.device, window.swapchain, UINT64_MAX, window.imageAvailableSemaphores[currentFrame_], VK_NULL_HANDLE, &imageIndex);

			imageIndices[i] = imageIndex;
			swapchains[i] = window.swapchain;
			waitStages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			imageAvailableSemaphores[i] = window.imageAvailableSemaphores[currentFrame_];
			renderFinishedSemaphores[i] = window.renderFinishedSemaphores[currentFrame_];

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = Window::windowInfos_.renderPass;
			renderPassInfo.framebuffer = window.framebuffers[imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = window.extent;

			VkClearValue clearColor = { {{0.5f, 0.0f, 1.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

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

			// Dessine l'UI

			VkCommandBuffer uiCommandBuffer = engine_.commandBuffers[i];
			vkResetCommandBuffer(uiCommandBuffer, 0);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(uiCommandBuffer, &beginInfo) != VK_SUCCESS)
			{
				throw - 1;
			}

			vkCmdBeginRenderPass(uiCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(uiCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(uiCommandBuffer, 0, 1, &scissor);

			vkCmdBindPipeline(uiCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline::Pipeline::pipelines_[0].pipeline);

			VkDeviceSize offsets = 0;
			vkCmdBindVertexBuffers(uiCommandBuffer, 0, 1, &engine_.vertexBuffer, &offsets);
			vkCmdBindIndexBuffer(uiCommandBuffer, engine_.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdEndRenderPass(uiCommandBuffer);

			if (vkEndCommandBuffer(uiCommandBuffer) != VK_SUCCESS)
			{
				throw - 1;
			}

			// Dessine les sprites

			for (int j = 0; j < Pipeline::pipelines_.size(); j++)
			{
				Pipeline::Pipeline_T pipeline = Pipeline::pipelines_[j];
				VkCommandBuffer commandBuffer = engine_.commandBuffers[i * Pipeline::pipelines_.size() + j + Window::windows_.size()];

				UBO ubo = { glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)), glm::perspective(glm::radians(60.0f), ((float)window.extent.width / (float)window.extent.height), 0.1f, 10.0f) };

				memcpy(engine_.uniformBufferMaps[currentFrame_], &ubo, sizeof(ubo));

				// Reset et enregistre le command buffer pour le rendu des entités

				vkResetCommandBuffer(commandBuffer, 0);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

				if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
				{
					throw - 1;
				}

				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline::pipelines_[j].pipeline);

				VkDeviceSize offsets = 0;
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &engine_.vertexBuffer, &offsets);
				vkCmdBindIndexBuffer(commandBuffer, engine_.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

				TypeList<VkDescriptorSet> descriptorSets = { engine_.descriptorSets[currentFrame_], engine_.bindlessDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline::pipelines_[j].pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, 6, Entity::getComponentPool<Sprite>().components.size(), 0, 0, 0);

				vkCmdEndRenderPass(commandBuffer);

				if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
				{
					throw - 1;
				}
			}
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.waitSemaphoreCount = imageAvailableSemaphores.size();
		submitInfo.pWaitSemaphores = imageAvailableSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.commandBufferCount = engine_.commandBuffers.size();
		submitInfo.pCommandBuffers = engine_.commandBuffers.data();

		submitInfo.signalSemaphoreCount = renderFinishedSemaphores.size();
		submitInfo.pSignalSemaphores = renderFinishedSemaphores.data();

		if (vkQueueSubmit(logicalDevice_.graphicsQueue, 1, &submitInfo, engine_.inFlightFences[currentFrame_]) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = renderFinishedSemaphores.size();
		presentInfo.pWaitSemaphores = renderFinishedSemaphores.data();

		presentInfo.swapchainCount = swapchains.size();
		presentInfo.pSwapchains = swapchains.data();
		presentInfo.pImageIndices = imageIndices.data();

		vkQueuePresentKHR(logicalDevice_.presentQueue, &presentInfo);

		currentFrame_ = (currentFrame_ + 1) % BLUEBERRY_MAX_FRAMES_IN_FLIGHT;
	}

	void Application::updateSprites(double dt)
	{
		for (Sprite& sprite : Entity::getComponentPool<Sprite>().components)
		{
			if (Texture::generations_[sprite.textureIndex_] != sprite.textureGeneration_)
			{
				// La texture n'est plus valide
				sprite.uvs_[0] = { 0,0,0 };
				sprite.uvs_[1] = { 0,0,0 };
				sprite.uvs_[2] = { 0,0,0 };
				sprite.uvs_[3] = { 0,0,0 };
				continue;
			}

			Texture::Texture_T& texture = Texture::textures_[sprite.textureIndex_];
			Vector4 uv = texture.getUV(dt);

			float x = uv.getX();
			float mx = x + uv.getZ();
			float y = uv.getY();
			float my = y + uv.getW();

			sprite.uvs_[0] = { x, y, 0 };
			sprite.uvs_[1] = { mx,	y, 0 };
			sprite.uvs_[2] = { mx,  my,  0 };
			sprite.uvs_[3] = { x, my,  0 };
		}
	}

	void Application::updateScripts(double dt)
	{
		for (Entity::ComponentPool<Script*> scriptPool : Entity::scriptPools.getValues())
		{
			for (Script* script : scriptPool.components)
			{
				if (script == nullptr) continue;

				script->update(dt);
			}
		}
	}

	void Application::init()
	{
		init("", 1, 0, 0, "");
	}

	void Application::init(const char* appName, int appMajorVersion, int appMinorVersion, int appPatchVersion, const char* defaultFolder)
	{
		if (!glfwInit()) return;
		createInstance(appName, appMajorVersion, appMinorVersion, appPatchVersion);

		defaultFolder_ = defaultFolder;
		physicalDevice_.device = VK_NULL_HANDLE;
	}

    void Application::terminate()
    {
		for (Window::Window_T window : Window::windows_)
		{
			glfwDestroyWindow(window.window);
		}

		if(logicalDevice_.device != VK_NULL_HANDLE) vkDeviceWaitIdle(logicalDevice_.device);

		if (physicalDevice_.device != VK_NULL_HANDLE)
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
			if (logicalDevice_.device != VK_NULL_HANDLE)
			{
				updateWindows();
				draw();
			}

			// Calcul du dt
			std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
			double dt = ((std::chrono::duration<double>)(currentTime - lastTime)).count();
			lastTime = currentTime;

			std::cout << 1 / dt << "\n";

			updateScripts(dt);
			updateSprites(dt);

            glfwPollEvents();
        }
    }

    void Application::stop()
    {
        isRunning_ = false;
    }
}