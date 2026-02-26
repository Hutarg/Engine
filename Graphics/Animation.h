#pragma once

#include "Texture.h"

namespace blueberry
{
	class Animation : public Texture
	{
	public:

		Animation();
		Animation(File textureFile, TypeList<Vector<5>> uvs); // Passer les uvs sous la forme x, y, w, h, t
		// Créer un nouveau type plus aproprié à la place de Vector5

	};
}