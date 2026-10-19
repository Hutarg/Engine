#pragma once

#include "Export.hpp"

#include "Utils/Shrub.hpp"

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
		static Shrub<Shader_T> shaders_;

		static Shrub<uint32_t> freeIndices_;
		static Shrub<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Pipeline;

	public:

		Shader() = default;
		Shader(const char* filename, ShaderType type, const char* stageName = "main");

		void destroy();

	};
}