#pragma once

#include "../api.h"

#include "../Maths/Functions.h"

#include <initializer_list>
#include <vector>
#include <ostream>

namespace blueberry
{
	template<typename T> class TypeList
	{
	private:

		T* pdata_ = nullptr;
		size_t size_ = 0;
		size_t capacity_ = 0;

	public:

		TypeList();
		TypeList(size_t size); 
		TypeList(std::initializer_list<T> list);
		TypeList(std::vector<T> list);
		TypeList(T pdata, size_t size);
		TypeList(T* pdata, size_t size);
		TypeList(const TypeList& other);
		TypeList(TypeList&& other) noexcept;
		~TypeList();

		void add(T value, int index = -1);
		void remove(int index = -1);

		T get(int index) const;
		void set(T value, int index);

		void resize(size_t size);
		void resize(size_t size, T value);
		void clear();

		T* data() const;
		size_t size() const;
		bool empty() const;

		bool contains(T value);
		int getCount(T value);
		int getIndex(T value);
		TypeList<int> getIndices(T value);

		T& operator[](int index) const;
		TypeList<T> operator+(TypeList<T>& other);
		void operator+=(TypeList<T> other);
		TypeList<T>& operator=(const TypeList<T>& other);
		TypeList<T>& operator=(TypeList<T>&& other) noexcept;
		bool operator==(TypeList<T> other);

		class Iterator
		{
		private:

			T* pdata_;

		public:

			Iterator(T* pdata);

			T& operator*();
			Iterator& operator++();
			bool operator!=(const Iterator& other);

		};

		Iterator begin() const;
		Iterator end() const;
	};

	template<typename T> inline TypeList<T>::TypeList()
	{
		pdata_ = nullptr;
		capacity_ = 0;
		size_ = 0;
	}

	template<typename T> inline TypeList<T>::TypeList(size_t size)
	{
		capacity_ = size;
		size_ = 0;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * size));

		if (std::is_default_constructible<T>::value)
		{
			for (size_t i = 0; i < size_; ++i)
			{
				new (&pdata_[i]) T();
			}

			size_ = size;
		}
	}

	template<typename T> inline TypeList<T>::TypeList(std::initializer_list<T> list)
	{
		capacity_ = list.size();
		size_ = capacity_;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(*(list.begin() + i));
		}
	}

	template<typename T> inline TypeList<T>::TypeList(std::vector<T> list)
	{
		capacity_ = list.capacity();
		size_ = list.size();
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(list[i]);
		}
	}

	template<typename T> inline TypeList<T>::TypeList(T value, size_t size)
	{
		capacity_ = size;
		size_ = size;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(value);
		}
	}

	template<typename T> inline TypeList<T>::TypeList(T* pdata, size_t size)
	{
		capacity_ = size;
		size_ = size;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(pdata[i]);
		}
	}

	template<typename T> inline TypeList<T>::TypeList(const TypeList& other)
	{
		size_ = other.size_;
		capacity_ = other.capacity_;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(other.pdata_[i]);
		}
	}

	template<typename T> inline TypeList<T>::TypeList(TypeList&& other) noexcept
	{
		pdata_ = other.pdata_;
		size_ = other.size_;
		capacity_ = other.capacity_;

		other.pdata_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	template<typename T> inline TypeList<T>::~TypeList()
	{
		clear();
	}

	template<typename T> inline void TypeList<T>::add(T value, int index)
	{
		if (index < 0) index += size_ + 1;
		if (index < 0 || index > size_) throw -1;

		if (capacity_ > size_)
		{
			for (int i = size_; i > index; i--)
			{
				new (&pdata_[i]) T(pdata_[i - 1]);
			}

			new (&pdata_[index]) T(value);
		}
		else
		{
			capacity_++;
			if(std::is_default_constructible<T>::value) capacity_ = max(capacity_, (size_t)index) * 2;

			T* pdata = static_cast<T*>(::operator new(sizeof(T) * capacity_));

			for (int i = 0; i < index; i++)
			{
				new (&pdata[i]) T(pdata_[i]);
				pdata_[i].~T();
			}

			new (&pdata[index]) T(value);

			for (int i = index; i < size_; i++)
			{
				new (&pdata[i + 1]) T(pdata_[i]);
				pdata_[i].~T();
			}

			::operator delete(pdata_);
			pdata_ = pdata;
		}

		size_++;
	}

	template<typename T> inline void TypeList<T>::remove(int index)
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;

		T* pdata = static_cast<T*>(::operator new(sizeof(T) * (size_ - 1)));

		for (int i = 0; i < index; i++)
		{
			new (&pdata[i]) T(pdata_[i]);
			pdata_[i].~T();
		}

		pdata_[index].~T();

		for (int i = index + 1; i < size_; i++)
		{
			new (&pdata[i - 1]) T(pdata_[i]);
			pdata_[i].~T();
		}

		::operator delete(pdata_);
		pdata_ = pdata;
		size_--;
	}

	template<typename T> inline T TypeList<T>::get(int index) const
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;
		return pdata_[index];
	}

	template<typename T> inline void TypeList<T>::set(T value, int index)
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;
		pdata_[index] = value;
	}

	template<typename T> inline void TypeList<T>::resize(size_t size)
	{
		T* pdata = static_cast<T*>(::operator new(sizeof(T) * size));
		size_t minSize = min(size, size_);

		for (size_t i = 0; i < minSize; i++)
		{
			new (&pdata[i]) T(pdata_[i]);
		}

		if (std::is_default_constructible<T>::value)
		{
			for (size_t i = minSize; i < size; ++i)
			{
				new (&pdata[i]) T();
			}
		}

		for (size_t i = 0; i < size_; i++)
		{
			pdata_[i].~T();
		}
		::operator delete(pdata_);

		capacity_ = size;
		size_ = size;

		pdata_ = pdata;
	}

	template<typename T> inline void TypeList<T>::resize(size_t size, T value)
	{
		T* pdata = static_cast<T*>(::operator new(sizeof(T) * size));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata[i]) T(pdata_[i]);
			pdata_[i].~T();
		}

		for (int i = size_; i < size; i++)
		{
			new (&pdata[i]) T(value);
		}

		capacity_ = size;
		size_ = size;

		if (pdata_ != nullptr) ::operator delete(pdata_);
		pdata_ = pdata;
	}

	template<typename T> inline void TypeList<T>::clear()
	{
		if (pdata_ != nullptr)
		{
			for(size_t i = 0; i < size_; i++) pdata_[i].~T();

			::operator delete(pdata_);
			pdata_ = nullptr;
		}

		capacity_ = 0;
		size_ = 0;
	}

	template<typename T> inline T* TypeList<T>::data() const
	{
		return pdata_;
	}

	template<typename T> inline size_t TypeList<T>::size() const
	{
		return size_;
	}

	template<typename T> inline bool TypeList<T>::empty() const
	{
		return size_ == 0;
	}

	template<typename T> inline bool TypeList<T>::contains(T value)
	{
		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] == value)
			{
				return true;
			}
		}
		
		return false;
	}

	template<typename T> inline int TypeList<T>::getCount(T value)
	{
		int count = 0;

		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] == value)
			{
				count++;
			}
		}

		return count;
	}

	template<typename T> inline int TypeList<T>::getIndex(T value)
	{
		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] == value)
			{
				return i;
			}
		}

		return -1;
	}

	template<typename T> inline TypeList<int> TypeList<T>::getIndices(T value)
	{
		TypeList<int> indices = {};

		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] == value)
			{
				indices.add(i);
			}
		}

		return indices;
	}

	template<typename T> inline T& TypeList<T>::operator[](int index) const
	{
		if (index < 0) index += static_cast<int>(size_);
		if (index < 0 || index >= size_) throw -1;
		return pdata_[index];
	}

	template<typename T> inline TypeList<T> TypeList<T>::operator+(TypeList<T>& other)
	{
		TypeList<T> newList = TypeList(size_ + other.size_);

		for (int i = 0; i < size_; i++)
		{
			new (&newList.pdata_[i]) T(pdata_[i]);
		}

		for (int i = 0; i < other.size_; i++)
		{
			new (&newList.pdata_[size_ + i]) T(other.pdata_[i]);
		}

		return newList;
	}

	template<typename T> inline void TypeList<T>::operator+=(TypeList<T> other)
	{
		size_t size = size_ + other.size_;
		T* pdata = static_cast<T*>(::operator new(sizeof(T) * size));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata[i]) T(pdata_[i]);
		}

		for (int i = 0; i < other.size_; i++)
		{
			new (&pdata[size_ + i]) T(other.pdata_[i]);
		}

		::operator delete(pdata_);
		pdata_ = pdata;
		size_ = size;
		capacity_ = size;
	}

	template<typename T> inline TypeList<T>& TypeList<T>::operator=(const TypeList<T>& other)
	{
		if (this == &other) return *this;

		clear();

		capacity_ = other.capacity_;
		size_ = other.size_;
		pdata_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

		for (int i = 0; i < size_; i++)
		{
			new (&pdata_[i]) T(other.pdata_[i]);
		}

		return *this;
	}

	template<typename T> inline TypeList<T>& TypeList<T>::operator=(TypeList<T>&& other) noexcept
	{
		if (this != &other)
		{
			T* pdata = pdata_;
			size_t size = size_;
			size_t capacity = capacity_;

			pdata_ = other.pdata_;
			size_ = other.size_;
			capacity_ = other.capacity_;

			other.pdata_ = pdata;
			other.size_ = size;
			other.capacity_ = capacity;
		}

		return *this;
	}
	
	template<typename T> inline bool TypeList<T>::operator==(TypeList<T> other)
	{
		if (size_ != other.size_) return false;

		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] != other.pdata_[i]) return false;
		}

		return true;
	}

	template<typename T> inline TypeList<T>::Iterator::Iterator(T* pdata)
	{
		pdata_ = pdata;
	}

	template<typename T> inline T& TypeList<T>::Iterator::operator*()
	{
		return *pdata_;
	}

	template<typename T> inline typename TypeList<T>::Iterator& TypeList<T>::Iterator::operator++()
	{
		pdata_++;
		return *this;
	}

	template<typename T> inline bool TypeList<T>::Iterator::operator!=(const Iterator& other)
	{
		return pdata_ != other.pdata_;
	}

	template<typename T> inline typename TypeList<T>::Iterator TypeList<T>::begin() const
	{
		return Iterator(pdata_);
	}

	template<typename T> inline typename TypeList<T>::Iterator TypeList<T>::end() const
	{
		return Iterator(pdata_ + size_);
	}
}