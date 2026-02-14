#include "Image.h"

#include "../Private/lodepng.h"

namespace blueberry
{
	Image::Image()
	{
		width_ = 0;
		height_ = 0;
		pixels_ = {};
	}

	Image::Image(const char* filename)
	{
		std::vector<unsigned char> pixels;
		if (lodepng::decode(pixels, width_, height_, filename)) throw std::runtime_error("Failed to decode the file");

		pixels_ = pixels;
	}

	Image::Image(int width, int height, TypeList<unsigned char> pixels)
	{
		width_ = width;
		height_ = height;

		pixels_ = pixels;
	}

	void Image::setWidth(int width, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		TypeList<unsigned char> pixels = TypeList<unsigned char>(width * height_ * 4);

		int index = 0;
		for (int y = 0; y < height_; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (x > width_)
				{
					pixels[index] = r;
					pixels[index + 1] = g;
					pixels[index + 2] = b;
					pixels[index + 3] = a;
				}
				else
				{
					pixels[index] = pixels_[y * width_ + x];
					pixels[index + 1] = pixels_[y * width_ + x + 1];
					pixels[index + 2] = pixels_[y * width_ + x + 2];
					pixels[index + 3] = pixels_[y * width_ + x + 3];
				}

				index += 4;
			}
		}

		width_ = width;
		pixels_ = pixels;
	}

	void Image::setHeight(int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		TypeList<unsigned char> pixels = TypeList<unsigned char>(width_ * height * 4);

		int index = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width_; x++)
			{
				if (y > height_)
				{
					pixels[index] = r;
					pixels[index + 1] = g;
					pixels[index + 2] = b;
					pixels[index + 3] = a;
				}
				else
				{
					int i = (y * width_ + x) * 4;
					pixels[index] = pixels_[i];
					pixels[index + 1] = pixels_[i + 1];
					pixels[index + 2] = pixels_[i + 2];
					pixels[index + 3] = pixels_[i + 3];
				}

				index += 4;
			}
		}

		height_ = height;
		pixels_ = pixels;
	}

	int Image::getWidth() const
	{
		return width_;
	}

	int Image::getHeight() const
	{
		return height_;
	}

	void Image::setSize(int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		TypeList<unsigned char> pixels = TypeList<unsigned char>(width * height * 4);

		int index = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width_; x++)
			{
				if (x > width_ or y > height_)
				{
					pixels[index] = r;
					pixels[index + 1] = g;
					pixels[index + 2] = b;
					pixels[index + 3] = a;
				}
				else
				{
					int i = (y * width_ + x) * 4;
					pixels[index] = pixels_[i];
					pixels[index + 1] = pixels_[i + 1];
					pixels[index + 2] = pixels_[i + 2];
					pixels[index + 3] = pixels_[i + 3];
				}

				index += 4;
			}
		}

		width_ = width;
		height_ = height;
		pixels_ = pixels;
	}

	void Image::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		int index = (y * width_ + x) * 4;
		pixels_[index] = r;
		pixels_[index + 1] = g;
		pixels_[index + 2] = b;
		pixels_[index + 3] = a;
	}

	void Image::setPixels(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		for (; y < height; y++)
		{
			for (; x < width; x++)
			{
				int index = (y * width_ + x) * 4;
				pixels_[index] = r;
				pixels_[index + 1] = g;
				pixels_[index + 2] = b;
				pixels_[index + 3] = a;
			}
		}
	}

	TypeList<unsigned char> Image::getPixels() const
	{
		return pixels_;
	}

	TypeList<unsigned char> Image::getPixels(int x, int y, int width, int height)
	{
		TypeList<unsigned char> pixels = TypeList<unsigned char>(width * height * 4);

		int index = 0;
		for (; y < height; y++)
		{
			for (; x < width; x++)
			{
				int i = (y * width_ + x) * 4;
				pixels[index] = pixels_[i];
				pixels[index + 1] = pixels_[i + 1];
				pixels[index + 2] = pixels_[i + 2];
				pixels[index + 3] = pixels_[i + 3];
			}

			index += 4;
		}

		return pixels;
	}

	Image Image::getImage(int x, int y, int width, int height)
	{
		return Image(width, height, getPixels(x, y, width, height));
	}

	TypeList<Image> Image::split(int width, int height)
	{
		return split(0, 0, width, height, width, height);
	}

	TypeList<Image> Image::split(int x, int y, int width, int height, int splitWidth, int splitHeight)
	{
		int widthCount = (width_ - x) / splitWidth;
		int heightCount = (height_ - y) / splitHeight;
		TypeList<Image> Images = TypeList<Image>(widthCount * heightCount);

		int index = 0;
		for (int h = 0; h < heightCount; h++)
		{
			for (int w = 0; w < widthCount; w++)
			{
				Images[index] = getImage(x + splitWidth * w, y + splitHeight * h, width, height);
				index++;
			}
		}

		return Images;
	}

	bool Image::operator==(Image other)
	{
		if (this == &other) return true;

		if (width_ != other.width_ or height_ != other.height_) return false;
		for (int i = 0; i < width_ * height_ * 4; i++) if (pixels_[i] != other.pixels_[i]) return false;
		return true;
	}
}