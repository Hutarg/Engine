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
		index_ = -1;
		generation_ = -1;
	}

	Texture::Texture(File file)
	{
		create(file);
	}

	Texture::Texture(Image image)
	{
		create(image);
	}

	void Texture::create(File file)
	{
		create(file.decode());
	}

	void Texture::create(Image image)
	{
		Texture_T texture_T = Texture_T{};
		texture_T.update = true;
		texture_T.getUV = [](double dt) { return Vector4{ 0,0,1,1 }; };

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

		// Agrandit si nécessaire le buffer de staging

		int pixelsSize = image.getWidth() * image.getHeight() * 4;

		if (pixelsSize > Application::engine_.stagingBufferSize)
		{
			Application::engine_.stagingBufferSize = pixelsSize;

			recreateBuffer(Application::physicalDevice_.device, Application::logicalDevice_.device, Application::engine_.stagingBuffer, Application::engine_.stagingBufferMemory,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Application::engine_.stagingBufferSize);

			vkBindBufferMemory(Application::logicalDevice_.device, Application::engine_.stagingBuffer, Application::engine_.stagingBufferMemory, 0);
		}

		void* data;
		vkMapMemory(Application::logicalDevice_.device, Application::engine_.stagingBufferMemory, 0, pixelsSize, 0, &data);
		memcpy(data, image.getPixels().data(), pixelsSize);
		vkUnmapMemory(Application::logicalDevice_.device, Application::engine_.stagingBufferMemory);

		transitionImageLayout(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, texture_T.image,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, Application::engine_.stagingBuffer,
			texture_T.image, static_cast<uint32_t>(image.getWidth()), static_cast<uint32_t>(image.getHeight()));
		transitionImageLayout(Application::logicalDevice_.device, Application::logicalDevice_.graphicsQueue, Application::engine_.commandPool, texture_T.image,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = texture_T.image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(Application::logicalDevice_.device, &imageViewCreateInfo, nullptr, &texture_T.imageView) != VK_SUCCESS)
		{
			throw - 1;
		}

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;

		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = Application::physicalDevice_.maxAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		if (vkCreateSampler(Application::logicalDevice_.device, &samplerCreateInfo, nullptr, &texture_T.sampler) != VK_SUCCESS)
		{
			throw - 1;
		}

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