#include "Animation.h"

#include "../Private/Structs.h"

namespace blueberry
{
	Animation::Animation() : Texture() {}

	Animation::Animation(File textureFile, TypeList<Vector4> uvs, float frameDuration)
	{
		Image image = textureFile.decode();
		create(image);

		for (int i = 0; i < uvs.size(); i++)
		{
			uvs[i][0] /= image.getWidth();
			uvs[i][1] /= image.getHeight();
			uvs[i][2] /= image.getWidth();
			uvs[i][3] /= image.getHeight();
		}

		Texture::textures_[this->index_].getUV = [uvs, frameDuration](double dt) -> Vector4
			{
				static int frame = 0;
				static double time = 0;

				time += dt;
				if (time > frameDuration)
				{
					time = 0;
					frame = (frame + 1) % uvs.size();
				}

				return uvs[frame];
			};
	}

	Animation::Animation(File textureFile, TypeList<Vector4> uvs, int(*getFrame)(float dt))
	{
		Image image = textureFile.decode();
		create(image);

		for (int i = 0; i < uvs.size(); i++)
		{
			uvs[i][0] /= image.getWidth();
			uvs[i][1] /= image.getHeight();
			uvs[i][2] /= image.getWidth();
			uvs[i][3] /= image.getHeight();
		}

		Texture::textures_[this->index_].getUV = [uvs, getFrame](double dt) -> Vector4
			{
				return uvs[getFrame(dt)];
			};
	}

	Animation::Animation(File textureFile, Vector4(*getUV)(float dt)) : Texture(textureFile)
	{
		Texture::textures_[this->index_].getUV = getUV;
	}

}