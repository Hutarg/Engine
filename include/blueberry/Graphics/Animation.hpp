#pragma once

#include "Export.hpp"

#include "Graphics/Texture.hpp"

#include "File/File.hpp"

#include "Maths/Vector.hpp"

namespace blueberry
{
	class Texture;

	class BLUEBERRY_API Animation : public Texture
	{
	private:

		friend class Sprite;

	public:

		Animation();
		Animation(File textureFile, Shrub<Vector4> uvs, float dt); // Passer les uvs sous la forme x, y, w, h
		Animation(File textureFile, Shrub<Vector4> uvs, int (*getFrame)(float dt));
		Animation(File textureFile, Vector4 (*func)(float dt));

	};
}