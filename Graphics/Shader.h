#pragma once

#include "../api.h"

#include "../Utils/TypeList.h"

namespace blueberry
{
	enum BLUEBERRY_API ShaderType
	{
		FRAGMENT,
		VERTEX,
	};

	class BLUEBERRY_API Shader
	{
	private:

		struct Shader_T;
		static TypeList<Shader_T> shaders_;

		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Pipeline;

	public:

		Shader() = default;
		Shader(const char* filename, ShaderType type, const char* stageName = "main");

		void destroy();

	};
}