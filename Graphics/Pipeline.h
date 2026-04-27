#pragma once

#include "../api.h"

#include "Shader.h"

namespace blueberry
{
	class BLUEBERRY_API Pipeline
	{
	private:

		struct Pipeline_T;
		static TypeList<Pipeline_T> pipelines_;

		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Application;

		friend class Sprite;

	public:

		Pipeline();
		Pipeline(TypeList<Shader>&& shaders);

	};
}