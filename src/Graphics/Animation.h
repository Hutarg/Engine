#pragma once

#include "../api.h"

#include "../Graphics/Texture.h"

#include "../File/File.h"

#include "../Maths/Vector.h"

namespace blueberry
{
	class Texture;

	class BLUEBERRY_API Animation : public Texture
	{
	private:

		friend class Sprite;

	public:

		Animation();
		Animation(File textureFile, TypeList<Vector4> uvs, float dt); // Passer les uvs sous la forme x, y, w, h
		Animation(File textureFile, TypeList<Vector4> uvs, int (*getFrame)(float dt));
		Animation(File textureFile, Vector4 (*func)(float dt));

	};
}