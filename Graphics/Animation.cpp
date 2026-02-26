#include "Animation.h"

#include "../Private/Structs.h"

namespace blueberry
{
	Animation::Animation() : Texture() {}

	Animation::Animation(File textureFile, TypeList<Vector<5>> uvs) : Texture(textureFile)
	{
		Texture::textures_[this->index_].uvs = uvs;
	}
}