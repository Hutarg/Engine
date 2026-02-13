#include "Texture.h"

#include "../Core/Application.h"

#include "../Private/Functions.h"
#include "../Private/Structs.h"

namespace blueberry
{
	TypeList<Texture::Texture_T> Texture::textures_ = {};
	TypeList<uint32_t> Texture::freeIndices_ = {};
	TypeList<uint32_t> Texture::generations_ = {};

	Texture::Texture()
	{
	}

	Texture::Texture(File file)
	{
		Texture_T texture_T = Texture_T{};

		Image image = file.decode();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = static_cast<uint32_t>(image.getWidth());
		imageCreateInfo.extent.height = static_cast<uint32_t>(image.getHeight());
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.flags = 0;

		if (vkCreateImage(Application::logicalDevice_.device, &imageCreateInfo, nullptr, &texture_T.image) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(Application::logicalDevice_.device, texture_T.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(Application::physicalDevice_.device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(Application::logicalDevice_.device, &allocInfo, nullptr, &texture_T.imageMemory) != VK_SUCCESS)
		{
			throw - 1;
		}

		vkBindImageMemory(Application::logicalDevice_.device, texture_T.image, texture_T.imageMemory, 0);

		transitionImageLayout(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, texture_T.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, Application::engine_.stagingBuffer, texture_T.image, static_cast<uint32_t>(image.getWidth()), static_cast<uint32_t>(image.getHeight()));
		transitionImageLayout(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, texture_T.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (freeIndices_.empty())
		{
			index_ = generations_.size();
			generation_ = 0;

			generations_.add(0);
			textures_.add(texture_T);
		}
		else
		{
			index_ = freeIndices_[-1];
			generation_ = generations_[index_];

			freeIndices_.remove(-1);
			textures_[index_] = texture_T;
		}
	}
}