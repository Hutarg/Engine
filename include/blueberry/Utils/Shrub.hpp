#pragma once

#include "Export.hpp"

#include "Maths/Functions.hpp"

#include <initializer_list>
#include <vector>
#include <ostream>

namespace blueberry
{
	template<typename T> class Shrub
	{
	private:

		T* pdata_ = nullptr;
		size_t size_ = 0;
		size_t capacity_ = 0;

	public:

		Shrub();
		Shrub(size_t size); 
		Shrub(std::initializer_list<T> list);
		Shrub(std::vector<T> list);
		Shrub(T* pdata, size_t size);
		Shrub(T pdata, size_t size);
		Shrub(const Shrub& other);
		Shrub(Shrub&& other) noexcept;
		~Shrub();

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

		int getIndex(T value);
		Shrub<int> getIndices(T value);
		bool contains(T value);
		int getCount(T value);

		T& operator[](int index) const;
		Shrub<T> operator+(Shrub<T>& other);
		Shrub<T>& operator+=(Shrub<T> other);
		Shrub<T>& operator=(const Shrub<T>& other);
		Shrub<T>& operator=(Shrub<T>&& other) noexcept;
		bool operator==(Shrub<T> other);

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

	/* Construit un Shrub de taille vide */
	template<typename T> inline Shrub<T>::Shrub()
	{
		pdata_ = nullptr;
		capacity_ = 0;
		size_ = 0;
	}

	/* Construit un Shrub avec une taille donnée si possible*/
	template<typename T> inline Shrub<T>::Shrub(size_t size)
	{
		if constexpr (!std::is_default_constructible_v<T>)
		{
			if (size != 0) throw -1;
		}

    	if (size == 0) 
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

		pdata_ = static_cast<T*>(::operator new(sizeof(T) * size));
    	size_t constructed = 0;

    	try
    	{
            for (; constructed < size; ++constructed)
			{
                new (&pdata_[constructed]) T();
			}
    	}
    	catch (...)
    	{
        	for (size_t i = 0; i < constructed; ++i)
			{
            	pdata_[--constructed].~T();
			}

        	::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

        	throw -1;
    	}

    	size_ = size;
    	capacity_ = size;
	}

	/* Construit un Shrub à partir d'une liste d'initialisation */
	template<typename T> inline Shrub<T>::Shrub(std::initializer_list<T> list)
	{
    	if (list.size() == 0) 
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

    	pdata_ = static_cast<T*>(::operator new(sizeof(T) * list.size()));
    	size_t constructed = 0;

    	try
    	{
    	    for (; constructed < list.size(); constructed++)
    	    {
    	        new (&pdata_[constructed]) T(*(list.begin() + constructed));
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata_[i].~T();
			}

    	    ::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

    	    throw;
    	}

    	size_ = list.size();
    	capacity_ = list.size();
	}

	/* Construit un Shrub à partir d'un vector */
	template<typename T> inline Shrub<T>::Shrub(std::vector<T> list)
	{
    	if (list.size() == 0) 
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

    	pdata_ = static_cast<T*>(::operator new(sizeof(T) * list.size()));
    	size_t constructed = 0;

    	try
    	{
    	    for (; constructed < list.size(); constructed++)
    	    {
    	        new (&pdata_[constructed]) T(list[constructed]);
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata_[i].~T();
			}

    	    ::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

    	    throw;
    	}

    	size_ = list.size();
    	capacity_ = list.size();
	}

	/* Construit un Shrub à partir d'un tableau */
	template<typename T> inline Shrub<T>::Shrub(T* pdata, size_t size)
	{
    	if (size == 0) 
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

    	pdata_ = static_cast<T*>(::operator new(sizeof(T) * size));
    	size_t constructed = 0;

    	try
    	{
    	    for (; constructed < size; constructed++)
    	    {
    	        new (&pdata_[constructed]) T(pdata[constructed]);
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata_[i].~T();
			}

    	    ::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

    	    throw;
    	}

    	size_ = size;
    	capacity_ = size;
	}

	/* Construit un Shrub avec une valeur donnée et une taille */
	template<typename T> inline Shrub<T>::Shrub(T value, size_t size)
	{
    	if (size == 0) 
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

    	pdata_ = static_cast<T*>(::operator new(sizeof(T) * size));
    	size_t constructed = 0;

    	try
    	{
    	    for (; constructed < size; constructed++)
    	    {
    	        new (&pdata_[constructed]) T(value);
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata_[i].~T();
			}

    	    ::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

    	    throw;
    	}

    	size_ = size;
    	capacity_ = size;
	}

	/* Copie un Shrub */
	template<typename T> inline Shrub<T>::Shrub(const Shrub<T>& other)
	{
		if (other.size_ == 0)
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return;
		};

    	pdata_ = static_cast<T*>(::operator new(sizeof(T) * other.capacity_));
    	size_t constructed = 0;

    	try
    	{
			for (; constructed < other.size_; constructed++)
    	    {
    	        new (&pdata_[constructed]) T(other.pdata_[constructed]);
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata_[i].~T();
			}

    	    ::operator delete(pdata_);
    	    pdata_ = nullptr;

			size_ = 0;
			capacity_ = 0;

    	    throw;
    	}

    	size_ = other.size_;
    	capacity_ = other.capacity_;
	}

	/* Copie un Shrub */
	template<typename T> inline Shrub<T>::Shrub(Shrub<T>&& other) noexcept
	{
		pdata_ = other.pdata_;
		size_ = other.size_;
		capacity_ = other.capacity_;

		other.pdata_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	/* Détruit le Shrub */
	template<typename T> inline Shrub<T>::~Shrub()
	{
		if (pdata_ != nullptr)
		{
			for(size_t i = 0; i < size_; i++) 
			{
				pdata_[i].~T();
			}

			::operator delete(pdata_);
		}
	}

	/* Ajoute un élément */
	template<typename T> inline void Shrub<T>::add(T value, int index)
	{
		if (index < 0) index += size_ + 1;
		if (index < 0 || index > size_) throw -1;


		if(capacity_ > size_)
		{
			size_t constructed = size_;

			try
			{
				/* Parcours le tableau dans le sens inverse pour éviter de recopier en boucle un élément */
				for (; constructed > index; constructed--)
				{
					new (&pdata_[constructed]) T(pdata_[constructed - 1]);
				}

				pdata_[index].~T();
				new (&pdata_[index]) T(value);
			}
			catch (...)
			{
				for (int i = 0; i < max(size_, constructed); i++)
				{
					pdata_[i].~T();
				}

    	    	::operator delete(pdata_);
				pdata_ = nullptr;
				size_ = 0;
				capacity_ = 0;

				throw -1;
			}
		}
		else
		{
			if constexpr (not std::is_default_constructible<T>::value) throw -1;
			capacity_ = capacity_ * 2 + 1;

			T* pdata = static_cast<T*>(::operator new(sizeof(T) * capacity_));
			size_t constructed = 0;

			try
			{
				/* Pas besoin de parcourir dans le sens inverse puisqu'on crée un nouveau tableau */
				for (; constructed < index; constructed++)
				{
					new (&pdata[constructed]) T(pdata_[constructed]);
				}

				new (&pdata[index]) T(value);
				constructed++;

				for (; constructed < size_ + 1; constructed++)
				{
					new (&pdata[constructed]) T(pdata_[constructed - 1]);
				}

				::operator delete(pdata_);
				pdata_ = pdata;
			}
			catch (...)
			{
				for (int i = 0; i < constructed; i++)
				{
					pdata[i].~T();
				}

				for(int i = 0; i < size_; i++)
				{
					pdata_[i].~T();
				}

				::operator delete(pdata);
				::operator delete(pdata_);
				pdata_ = nullptr;

				throw -1;
			}
		}

		size_++;
	}

	/*  */
	template<typename T> inline void Shrub<T>::remove(int index)
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;

		size_t constructed = index;

		try
		{
			for (int i = index + 1; i < size_; i++)
			{
				new (&pdata_[i - 1]) T(pdata_[i]);
				constructed++;
			}
		}
		catch(...)
		{
			for (size_t i = 0; i < constructed; i++)
			{
				pdata_[i].~T();
			}

			::operator delete(pdata_);
			throw -1;
		}

		pdata_[size_ - 1].~T();
		size_--;
	}

	template<typename T> inline T Shrub<T>::get(int index) const
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;
		return pdata_[index];
	}

	template<typename T> inline void Shrub<T>::set(T value, int index)
	{
		if (index < 0) index += size_;
		if (index < 0 || index >= size_) throw -1;
		pdata_[index] = value;
	}

	template<typename T> inline void Shrub<T>::resize(size_t size)
	{
		if (size == size_) return;

		if (size == 0)
		{
			clear();
			return;
		}

		if constexpr (not std::is_default_constructible<T>::value && size > size_) throw -1;

		T* pdata = static_cast<T*>(::operator new(sizeof(T) * size));
		size_t constructed = 0;

		try
		{
			for (; constructed < size_; constructed++)
			{
				new (&pdata[constructed]) T(pdata_[constructed]);
			}

			for (; constructed < size; constructed++)
			{
				new (&pdata[constructed]) T();
			}
		}
		catch (...)
		{
			for (int i = 0; i < constructed; i++)
			{
				pdata[i].~T();
			}

			for(int i = 0; i < size_; i++)
			{
				pdata_[i].~T();
			}

			::operator delete(pdata);
			::operator delete(pdata_);
			pdata_ = nullptr;

			throw -1;
		}

		for(size_t i = 0; i < size_; i++)
		{
			pdata_[i].~T();
		}

		::operator delete(pdata_);
		pdata_ = pdata;

		size_ = size;
		capacity_ = size;
	}

	template<typename T> inline void Shrub<T>::resize(size_t size, T value)
	{
		if(size == size_) return;

		if(size == 0)
		{
			clear();
			return;
		}

		T* pdata = static_cast<T*>(::operator new(sizeof(T) * size));
		size_t constructed = 0;

		try
		{
			for (; constructed < size_; constructed++)
			{
				new (&pdata[constructed]) T(pdata_[constructed]);
			}

			for (; constructed < size; constructed++)
			{
				new (&pdata[constructed]) T(value);
			}
		}
		catch (...)
		{
			for (int i = 0; i < constructed; i++)
			{
				pdata[i].~T();
			}

			for(int i = 0; i < size_; i++)
			{
				pdata_[i].~T();
			}

			::operator delete(pdata);
			::operator delete(pdata_);
			pdata_ = nullptr;

			throw -1;
		}

		for(size_t i = 0; i < size_; i++)
		{
			pdata_[i].~T();
		}

		::operator delete(pdata_);
		pdata_ = pdata;

		size_ = size;
		capacity_ = size;
	}

	template<typename T> inline void Shrub<T>::clear()
	{
		if (pdata_ != nullptr)
		{
			for(size_t i = 0; i < size_; i++) 
			{
				pdata_[i].~T();
			}
		}

		size_ = 0;
	}

	template<typename T> inline T* Shrub<T>::data() const
	{
		return pdata_;
	}

	template<typename T> inline size_t Shrub<T>::size() const
	{
		return size_;
	}

	template<typename T> inline bool Shrub<T>::empty() const
	{
		return size_ == 0;
	}

	template<typename T> inline int Shrub<T>::getIndex(T value)
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

	template<typename T> inline Shrub<int> Shrub<T>::getIndices(T value)
	{
		Shrub<int> indices = {};

		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] == value)
			{
				indices.add(i);
			}
		}

		return indices;
	}

	template<typename T> inline bool Shrub<T>::contains(T value)
	{
		if(getIndex(value) != -1) return true;
		return false;
	}

	template<typename T> inline int Shrub<T>::getCount(T value)
	{
		return getIndices(value).size();
	}

	template<typename T> inline T& Shrub<T>::operator[](int index) const
	{
		if (index < 0) index += static_cast<int>(size_);
		if (index < 0 || index >= size_) throw -1;
		return pdata_[index];
	}

	template<typename T> inline Shrub<T> Shrub<T>::operator+(Shrub<T>& other)
	{
		Shrub<T> newList = Shrub();
		newList.size_ = size_ + other.size_;
		newList.capacity_ = newList.size_;

		if (newList.size_ == 0) return newList;

		T* pdata = static_cast<T*>(::operator new(sizeof(T) * newList.capacity_));
		size_t constructed = 0;

		try
		{
			for (int i = 0; i < size_; i++)
			{
				new (&newList.pdata[i]) T(pdata_[i]);
				constructed++;
			}

			for (int i = 0; i < other.size_; i++)
			{
				new (&pdata_[size_ + i]) T(other.pdata_[i]);
				constructed++;
			}

		}
		catch (...)
		{
			for (int i = 0; i < constructed; i++)
			{
				newList.pdata_[i].~T();
			}

			::operator delete(newList.pdata_);
			newList.pdata_ = nullptr;
			newList.size_ = 0;
			newList.capacity_ = 0;

			throw -1;
		}

		return newList;
	}

	template<typename T> inline Shrub<T>& Shrub<T>::operator+=(Shrub<T> other)
	{
		size_t newSize = size_ + other.size_;
		size_t constructed = 0;

		T* pdata = static_cast<T*>(::operator new(sizeof(T) * newSize));

		try
		{
			for (int i = 0; i < size_; i++)
			{
				new (&pdata[i]) T(pdata_[i]);
				constructed++;
			}

			for (int i = 0; i < other.size_; i++)
			{
				new (&pdata[size_ + i]) T(other.pdata_[i]);
				constructed++;
			}
		}
		catch (...)
		{
			for (int i = 0; i < constructed; i++)
			{
				pdata[i].~T();
			}

			::operator delete(pdata);
			throw -1;
		}

		::operator delete(pdata_);
		pdata_ = pdata;

		size_ = size;
		capacity_ = size;

		return *this;
	}

	template<typename T> inline Shrub<T>& Shrub<T>::operator=(const Shrub<T>& other)
	{
		if (other.size_ == 0)
		{
			pdata_ = nullptr;
			capacity_ = 0;
			size_ = 0;
			return *this;
		};

    	T* pdata = static_cast<T*>(::operator new(sizeof(T) * other.capacity_));
    	size_t constructed = 0;

    	try
    	{
			for (; constructed < other.size_; constructed++)
    	    {
    	        new (&pdata[constructed]) T(other.pdata_[constructed]);
    	    }
    	}
    	catch (...)
    	{
    	    for (size_t i = 0; i < constructed; i++)
			{
    	        pdata[i].~T();
			}

    	    ::operator delete(pdata);
    	    throw;
    	}

		pdata_ = pdata;
    	size_ = other.size_;
    	capacity_ = other.capacity_;

		return *this;
	}

	template<typename T> inline Shrub<T>& Shrub<T>::operator=(Shrub<T>&& other) noexcept
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
	
	template<typename T> inline bool Shrub<T>::operator==(Shrub<T> other)
	{
		if (size_ != other.size_) return false;

		for (int i = 0; i < size_; i++)
		{
			if (pdata_[i] != other.pdata_[i]) return false;
		}

		return true;
	}

	template<typename T> inline Shrub<T>::Iterator::Iterator(T* pdata)
	{
		pdata_ = pdata;
	}

	template<typename T> inline T& Shrub<T>::Iterator::operator*()
	{
		return *pdata_;
	}

	template<typename T> inline typename Shrub<T>::Iterator& Shrub<T>::Iterator::operator++()
	{
		pdata_++;
		return *this;
	}

	template<typename T> inline bool Shrub<T>::Iterator::operator!=(const Iterator& other)
	{
		return pdata_ != other.pdata_;
	}

	template<typename T> inline typename Shrub<T>::Iterator Shrub<T>::begin() const
	{
		return Iterator(pdata_);
	}

	template<typename T> inline typename Shrub<T>::Iterator Shrub<T>::end() const
	{
		return Iterator(pdata_ + size_);
	}
}