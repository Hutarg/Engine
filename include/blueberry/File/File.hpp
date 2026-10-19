#pragma once

#include "Export.hpp"

#include "Utils/String.hpp"
#include "Utils/Image.hpp"
#include "Utils/Shrub.hpp"

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