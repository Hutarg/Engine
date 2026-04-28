#include "Shader.h"

#include <fstream>

#include "../Private/Structs.h"

#include "../Utils/TypeList.h"

namespace blueberry
{
	TypeList<Shader::Shader_T> Shader::shaders_ = {};
	TypeList<uint32_t> Shader::freeIndices_ = {};
	TypeList<uint32_t> Shader::generations_ = {};

	Shader::Shader(const char* filename, ShaderType type, const char* stageName)
	{
		Shader_T shader_T = {};
		shader_T.stageName = stageName;

		switch (type)
		{
		case blueberry::FRAGMENT:
			shader_T.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case blueberry::VERTEX:
			shader_T.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		default:
			break;
		}

		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open()) throw - 1;

		size_t size = file.tellg();
		TypeList<char> buffer(size);

		file.seekg(0);
		file.read(buffer.data(), size);
		file.close();

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = buffer.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

		if (vkCreateShaderModule(Application::logicalDevice_.device, &shaderModuleCreateInfo, nullptr, &shader_T.shader))
		{
			throw - 1;
		}

		if (freeIndices_.empty())
		{
			index_ = generations_.size();
			generation_ = 0;

			generations_.add(0);
			shaders_.add(shader_T);
		}
		else
		{
			index_ = freeIndices_[-1]; // Prend le dernier élément pour remove en O(1)
			generation_ = generations_[index_];

			freeIndices_.remove(-1);
			shaders_[index_] = shader_T;
		}
	}

	void Shader::destroy()
	{
		if (index_ > 0 && generations_[index_] == generation_)
		{
			vkDestroyShaderModule(Application::logicalDevice_.device, shaders_[index_].shader, nullptr);
			generations_[index_]++;

			index_ = -1;
			generation_ = -1;
		}
	}
}