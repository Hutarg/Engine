#pragma once

#include "../api.h"

#include "../Utils/String.h"
#include "../Utils/Image.h"
#include "../Utils/TypeList.h"

namespace blueberry
{
	class Image;

	class BLUEBERRY_API File
	{
	private:

		const char* filename_;

	public:

		File(const char* filename);

		String read();
		void write(const char* text);

		Image decode();
		void encode(Image image);

	};
}