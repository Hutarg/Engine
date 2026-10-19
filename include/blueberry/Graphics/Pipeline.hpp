#pragma once

#include "Export.hpp"

#include "Shader.hpp"

namespace blueberry
{
	enum PipelineType
	{
		SPRITE,
		GUI
	};

	class BLUEBERRY_API Pipeline
	{
	private:

		struct Pipeline_T;
		static Shrub<Pipeline_T> pipelines_;

		static Shrub<uint32_t> freeIndices_;
		static Shrub<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Application;

		friend class Sprite;

	public:

		Pipeline();
		Pipeline(Shrub<Shader>&& shaders, PipelineType type);

	};
}