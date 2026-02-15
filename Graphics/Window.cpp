#include "Window.h"

#include "../Private/Structs.h"
#include "../Private/Functions.h"

#include "../Core/Application.h"

bool operator==(const VkSurfaceFormatKHR& a, const VkSurfaceFormatKHR& b)
{
	return a.format == b.format and a.colorSpace == b.colorSpace;
}

namespace blueberry
{
	TypeList<uint32_t> Window::freeIndices_ = {};
	TypeList<uint32_t> Window::generations_ = {};
	TypeList<Window::Window_T> Window::windows_ = {};

	Window::WindowInfos_T Window::windowInfos_ = {};

	Window::Window(const char* title, uint32_t width, uint32_t height)
	{
		Window_T window_T = {};

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window_T.window = glfwCreateWindow(width, height, title, NULL, NULL);

		if (window_T.window == NULL)
		{
			throw -1;
		}

		glfwSetWindowUserPointer(window_T.window, &window_T);
		glfwSetWindowSizeCallback(window_T.window, Functions::windowSizeCallback);

		if (glfwCreateWindowSurface(Application::instance_.instance, window_T.window, nullptr, &window_T.surface) != VK_SUCCESS)
		{
			throw - 1;
		}

		TypeList<VkSurfaceFormatKHR> availableFormats;
		TypeList<VkPresentModeKHR> availablePresentModes;

		if (Application::physicalDevice_.device == VK_NULL_HANDLE)
		{
			// Crée les membres vulkan qui nécessitent une surface (fenêtre) pour être créer lors de la création de la première fenêtre

			// Choisis le premier gpu adapté (ils sont déjà rangés dans l'ordre du "meilleur" au "pire")

			TypeList<Application::PhysicalDevice_T> physicalDevices = Application::getPhysicalDevices();

			for (Application::PhysicalDevice_T physicalDevice : physicalDevices)
			{
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.device, window_T.surface, &window_T.capabilities);

				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.device, window_T.surface, &formatCount, nullptr);

				availableFormats = TypeList<VkSurfaceFormatKHR>(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.device, window_T.surface, &formatCount, availableFormats.data());

				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.device, window_T.surface, &presentModeCount, nullptr);

				availablePresentModes = TypeList<VkPresentModeKHR>(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.device, window_T.surface, &presentModeCount, availablePresentModes.data());

				if (!availableFormats.empty() and !availablePresentModes.empty())
				{
					Application::physicalDevice_ = physicalDevice;
					break;
				}
			}

			if (Application::physicalDevice_.device == VK_NULL_HANDLE)
			{
				throw -1;
			}

			Application::createLogicalDevice();
			Application::createEngine();

			// Cherche le meilleur format. Normalement un même format est supporté par toutes les fenêtres. Généraliser un format à toutes les fenêtres
			// permet d'éviter d'utiliser une même pipeline pour chaque fenêtre et éviter de la dupliquer (ce qui peut être couteux)

			// Cherche le meilleur format (pt mettre une note en fonction des formats avec un tableau où l'index est le format et tableau[index] est la note du format)
			windowInfos_.format = availableFormats[0];
			for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					windowInfos_.format = availableFormat;
					break;
				}
			}

			// Cherche le meilleur mode de présentation
			windowInfos_.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					windowInfos_.presentMode = availablePresentMode;
					break;
				}

				if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					windowInfos_.presentMode = availablePresentMode;
					break;
				}
			}

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = windowInfos_.format.format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(Application::logicalDevice_.device, &renderPassInfo, nullptr, &windowInfos_.renderPass) != VK_SUCCESS)
			{
				throw - 1;
			}
		}
		else
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Application::physicalDevice_.device, window_T.surface, &window_T.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(Application::physicalDevice_.device, window_T.surface, &formatCount, nullptr);

			availableFormats = TypeList<VkSurfaceFormatKHR>(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(Application::physicalDevice_.device, window_T.surface, &formatCount, availableFormats.data());

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(Application::physicalDevice_.device, window_T.surface, &presentModeCount, nullptr);

			availablePresentModes = TypeList<VkPresentModeKHR>(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Application::physicalDevice_.device, window_T.surface, &presentModeCount, availablePresentModes.data());
		}

		// Si une fenêtre ne supporte pas le même format ou mode de présentation, on ne peut pas utiliser de pipeline
		// A changer dans le futur pour que ca ne résulte plus en une erreur mais de toute façon il vaut mieux éviter plusieurs fenêtres
		if (!availableFormats.contains(windowInfos_.format) or !availablePresentModes.contains(windowInfos_.presentMode))
		{
			throw - 1;
		}

		if (window_T.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			window_T.extent = window_T.capabilities.currentExtent;
		}
		else 
		{
			int width, height;
			glfwGetWindowSize(window_T.window, &width, &height);

			window_T.extent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			window_T.extent.width = clamp(window_T.extent.width, window_T.capabilities.minImageExtent.width, window_T.capabilities.maxImageExtent.width);
			window_T.extent.height = clamp(window_T.extent.height, window_T.capabilities.minImageExtent.height, window_T.capabilities.maxImageExtent.height);
		}

		uint32_t imageCount = window_T.capabilities.minImageCount + 1;

		if (window_T.capabilities.maxImageCount > 0 && imageCount > window_T.capabilities.maxImageCount)
		{
			imageCount = window_T.capabilities.maxImageCount;
		}

		// Crée la swapchain
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = window_T.surface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = windowInfos_.format.format;
		swapchainCreateInfo.imageColorSpace = windowInfos_.format.colorSpace;
		swapchainCreateInfo.imageExtent = window_T.extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { Application::physicalDevice_.graphicsQueueIndex, Application::physicalDevice_.presentQueueIndex };
		if (Application::logicalDevice_.graphicsQueue != Application::logicalDevice_.presentQueue)
		{
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		swapchainCreateInfo.preTransform = window_T.capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = windowInfos_.presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(Application::logicalDevice_.device, &swapchainCreateInfo, nullptr, &window_T.swapchain) != VK_SUCCESS)
		{
			throw - 1;
		}

		// Récupère les images de la swapchain
		vkGetSwapchainImagesKHR(Application::logicalDevice_.device, window_T.swapchain, &imageCount, nullptr);

		window_T.images = TypeList<VkImage>(imageCount);
		vkGetSwapchainImagesKHR(Application::logicalDevice_.device, window_T.swapchain, &imageCount, window_T.images.data());

		window_T.imageViews = TypeList<VkImageView>(imageCount);

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

		for (int i = 0; i < window_T.images.size(); i++)
		{
			imageViewCreateInfo.image = window_T.images[i];
			imageViewCreateInfo.format = windowInfos_.format.format;

			if (vkCreateImageView(Application::logicalDevice_.device, &imageViewCreateInfo, nullptr, &window_T.imageViews[i]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		window_T.framebuffers = TypeList<VkFramebuffer>(window_T.imageViews.size());
		for (size_t i = 0; i < window_T.imageViews.size(); i++)
		{
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = windowInfos_.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &window_T.imageViews[i];
			framebufferInfo.width = window_T.extent.width;
			framebufferInfo.height = window_T.extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(Application::logicalDevice_.device, &framebufferInfo, nullptr, &window_T.framebuffers[i]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		window_T.imageAvailableSemaphores = TypeList<VkSemaphore>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);
		window_T.renderFinishedSemaphores = TypeList<VkSemaphore>(BLUEBERRY_MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < BLUEBERRY_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			if (vkCreateSemaphore(Application::logicalDevice_.device, &semaphoreInfo, nullptr, &window_T.imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(Application::logicalDevice_.device, &semaphoreInfo, nullptr, &window_T.renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw - 1;
			}
		}

		if (freeIndices_.empty())
		{
			index_ = generations_.size();
			generation_ = 0;

			generations_.add(0);
			windows_.add(window_T);
		}
		else
		{
			index_ = freeIndices_[-1]; // Prend le dernier élément pour remove en O(1)
			generation_ = generations_[index_];

			freeIndices_.remove(-1);
			windows_[index_] = window_T;
		}
	}

	void Window::close() const
	{
		if (isClosed()) return;

		glfwDestroyWindow(windows_[index_].window);

		freeIndices_.add(index_);
		generations_[index_]++;
	}

	bool Window::shouldClose() const
	{
		if (isClosed()) return false;

		return glfwWindowShouldClose(windows_[index_].window);
	}

	bool Window::isClosed() const
	{
		if (index_ == -1) return true;
		if (generations_[index_] != generation_) return true;
		return false;
	}

	uint32_t Window::getWidth() const
	{
		if (isClosed()) return 0;
		
		int width;
		glfwGetWindowSize(windows_[index_].window, &width, nullptr);
		return width;
	}

	uint32_t Window::getHeight() const
	{
		if (isClosed()) return 0;

		int height;
		glfwGetWindowSize(windows_[index_].window, nullptr, &height);
		return height;
	}

	Vector2 Window::getSize() const
	{
		if (isClosed()) return Vector2();

		int width, height;
		glfwGetWindowSize(windows_[index_].window, &width, &height);
		return Vector2(width, height);
	}
}