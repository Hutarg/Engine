#pragma once

#include "../blueberry.h"
#include "../File/File.h"

namespace blueberry
{
	class BLUEBERRY_API Texture
	{
	private:

		struct Texture_T;

		static TypeList<Texture_T> textures_;
		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

	public:

		Texture();
		Texture(File file);

	};
}