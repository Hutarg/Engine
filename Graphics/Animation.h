#pragma once

#include "Texture.h"
#include "../Utils/Tuple.h"

namespace blueberry
{
	class Animation : public Texture
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