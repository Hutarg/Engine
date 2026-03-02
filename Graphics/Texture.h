#pragma once

#include "../blueberry.h"

namespace blueberry
{
	class File;
	class Image;

	class BLUEBERRY_API Texture
	{
	protected:

		struct Texture_T;

		static TypeList<Texture_T> textures_;
		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Application;
		friend class Sprite;

	public:

		Texture();
		Texture(File file);
		Texture(Image image);

		void create(File file);
		void create(Image image);

	};
}