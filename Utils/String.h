#pragma once

#include "../api.h"

#include <cstring>

namespace blueberry
{
	class BLUEBERRY_API String
	{
	private:

		char* data_;
		size_t size_;

	public:

		String();
		String(size_t size);
		String(const char* str);
		String(const String& other);

		char* str();
		const char* cStr();

		char& operator[](size_t index);

	};

	inline String::String()
	{
		size_ = 0;
		data_ = new char[1];
		data_[0] = '\0';
	}

	inline String::String(size_t size)
	{
		size_ = size;
		data_ = new char[size + 1];
		data_[size] = '\0';
	}

	inline String::String(const char* str)
	{
		if (!str)
		{
			size_ = 0;
			data_ = new char[1];
			data_[0] = '\0';
		}
		else
		{
			size_ = std::strlen(str);
			data_ = new char[size_ + 1];
			std::memcpy(data_, str, size_ + 1);
		}
	}

	inline String::String(const String& other)
	{
		size_ = other.size_;
		data_ = new char[size_ + 1];
		std::memcpy(data_, other.data_, size_ + 1);
	}

	inline char* String::str()
	{
		return data_;
	}

	inline const char* String::cStr()
	{
		return data_;
	}

	inline char& String::operator[](size_t index)
	{
		return data_[index];
	}
}