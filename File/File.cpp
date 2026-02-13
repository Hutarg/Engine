#include "File.h"

#include "../Private/lodepng.h"

#include <fstream>

namespace blueberry
{
	File::File(const char* filename)
	{
		filename_ = filename;
	}

	String File::read()
	{
		std::ifstream file(filename_, std::ios::binary | std::ios::ate);

		if (!file.is_open()) throw - 1;

		size_t size = file.tellg();
		String string(size);

		file.seekg(0);
		file.read(string.str(), size);
		file.close();

		return string;
	}

	void File::write(const char* text)
	{
		std::ofstream file(filename_);

		if (!file.is_open()) throw - 1;

		file << text;
		file.close();
	}

	Image File::decode()
	{
		unsigned width, height;
		std::vector<unsigned char> pixels;
		if (lodepng::decode(pixels, width, height, filename_)) throw - 1;

		return Image();
	}

	void File::encode(Image image)
	{
		if (lodepng::encode(std::string(filename_), std::vector<unsigned char>(image.getPixels().data(), image.getPixels().data() + image.getPixels().size()), image.getWidth(), image.getHeight())) throw - 1;
	}
}
