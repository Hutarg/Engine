#pragma once

#include "../blueberry.h"

#include "../Utils/String.h"
#include "../Utils/Image.h"

namespace blueberry
{
	class BLUEBERRY_API File
	{
	private:

		const char* filename_;

	public:

		explicit File(const char* filename);

		String read();
		void write(const char* text);

		Image decode();
		void encode(Image image);

	};
}