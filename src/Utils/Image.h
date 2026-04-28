#pragma once

#include "../api.h"

#include "../Utils/TypeList.h"

namespace blueberry
{
	class BLUEBERRY_API Image
	{
	private:

		unsigned width_ = 0, height_ = 0;
		TypeList<unsigned char> pixels_;

	public:

		Image();
		Image(const char* filename);
		Image(int width, int height, TypeList<unsigned char> pixels);

		void setWidth(int width, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);
		void setHeight(int height, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);

		int getWidth() const;
		int getHeight() const;

		void setSize(int width, int height, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);
		void setPixel(int x, int y, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);
		void setPixels(int x, int y, int width, int height, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);

		TypeList<unsigned char> getPixels() const;
		TypeList<unsigned char> getPixels(int x, int y, int width, int height);

		Image getImage(int x, int y, int width, int height);

		TypeList<Image> split(int width, int height);
		TypeList<Image> split(int x, int y, int width, int height, int splitWidth, int splitHeight);

		bool operator==(Image other);

	};
}