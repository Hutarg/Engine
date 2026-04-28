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

		size_t getSize();

		char* str();
		const char* cStr();

		char& operator[](size_t index);
		String operator+(String str);
		String operator+(const char* str);

	};

	inline String operator+(const char* str1, String str2)
	{
		size_t size = strlen(str1) + str2.getSize() + 1;
		char* result = new char[size];

		strcpy_s(result, size, str1);
		strcat_s(result, size, str2.str());

		return String(result);
	}

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

	inline size_t String::getSize()
	{
		return size_;
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

	inline String String::operator+(String str)
	{
		return String();
	}

	inline String String::operator+(const char* str)
	{
		size_t size = size_ + strlen(str) + 1;
		char* result = new char[size];

		strcpy_s(result, size, data_);
		strcat_s(result, size, str);

		return String(result);
	}
}