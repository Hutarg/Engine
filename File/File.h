#pragma once

#include "../blueberry.h"

#include "../Utils/String.h"
#include "../Utils/Image.h"

namespace blueberry
{
	class String;
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