#pragma once

#include <initializer_list>

#include "TypeList.h"

namespace blueberry
{
	template<typename Key, typename Value> class Map
	{
	private:

		Key* pkeys_ = nullptr;
		Value* pvalues_ = nullptr;
		size_t size_;
		size_t capacity_;

		int getIndex(Key key);

	public:

		Map();
		Map(const Map& other);
		Map(Map&& other);
		~Map();

		void add(Key key, Value value);
		void remove(Key key);

		TypeList<Key> getKeys();

		class Iterator
		{
		private:

			Key* pkeys_;
			Value* pvalues_;
			int index_;

		public:

			Iterator(Key* pkeys, Value* pvalues, int index);

			Iterator& operator++();
			bool operator!=(const Iterator& other);

		};

		Iterator begin();
		Iterator end();

		size_t size();
		bool empty();

		Value& operator[](Key key);
		Map<Key, Value>& operator=(const Map<Key, Value>& other);
		Map<Key, Value>& operator=(Map<Key, Value>&& other);

	};

	template<typename Key, typename Values> inline int Map<Key, Values>::getIndex(Key key)
	{
		int index = -1;
		for (int i = 0; i < size_; i++)
		{
			if (pkeys_[i] == key)
			{
				index = i;
				break;
			}
		}

		return index;
	}

	template<typename Key, typename Value> inline Map<Key, Value>::Map()
	{
		size_ = 0;
		capacity_ = 0;
		pkeys_ = nullptr;
		pvalues_ = nullptr;
	}

	template<typename Key, typename Value> inline Map<Key, Value>::Map(const Map& other)
	{
		size_ = other.size_;
		capacity_ = other.capacity_;

		pkeys_ = static_cast<Key*>(::operator new(sizeof(Key) * capacity_)); 
		pvalues_ = static_cast<Value*>(::operator new(sizeof(Value) * capacity_));

		for (int i = 0; i < capacity_; i++)
		{
			new (&pkeys_[i]) Key(other.pkeys_[i]);
			new (&pvalues_[i]) Value(other.pvalues_[i]);
		}
	}

	template<typename Key, typename Value> inline Map<Key, Value>::Map(Map&& other)
	{
		pkeys_ = other.pkeys_;
		pvalues_ = other.pvalues_;
		size_ = other.size_;
		capacity_ = other.capacity_;

		other.pkeys_ = nullptr;
		other.pvalues_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	template<typename Key, typename Value> inline Map<Key, Value>::~Map()
	{
		if (pkeys_ != nullptr)
		{
			for (size_t i = 0; i < size_; i++)
			{
				pkeys_[i].~Key();
				pvalues_[i].~Value();
			}

			::operator delete(pkeys_);
			::operator delete(pvalues_);
		}

		pkeys_ = nullptr;
	}

	template<typename Key, typename Value> inline void Map<Key, Value>::add(Key key, Value value)
	{
		if(capacity_ == size_)
		{
			capacity_ = capacity_ * 2 + 1;
			Key* pkeys = static_cast<Key*>(::operator new(sizeof(Key) * capacity_));
			Value* pvalues = static_cast<Value*>(::operator new(sizeof(Value) * capacity_));

			for (int i = 0; i < size_; i++)
			{
				new (&pkeys[i]) Key(pkeys_[i]);
				new (&pvalues[i]) Value(pvalues_[i]);

				pkeys_[i].~Key();
				pvalues_[i].~Value();
			}

			::operator delete(pkeys_);
			::operator delete(pvalues_);

			pkeys_ = pkeys;
			pvalues_ = pvalues;
		}

		new (&pkeys_[size_]) Key(key);
		new (&pvalues_[size_]) Value(value);

		size_++;
	}

	template<typename Key, typename Value> inline void Map<Key, Value>::remove(Key key)
	{
		int index = getIndex(key);
		if (index == -1) return;

		for (int i = index; i < size_ - 1; i++)
		{
			pkeys_[i] = pkeys_[i + 1];
			pvalues_[i] = pvalues_[i + 1];
		}

		size_--;
	}

	template<typename Key, typename Value> inline TypeList<Key> Map<Key, Value>::getKeys()
	{
		return TypeList<Key>(pkeys_, size_);
	}

	template<typename Key, typename Value> inline Map<Key, Value>::Iterator::Iterator(Key* pkeys, Value* pvalues, int index)
	{
		pkeys_ = pkeys;
		pvalues_ = pvalues;
		index_ = index;
	}

	template<typename Key, typename Value> inline typename Map<Key, Value>::Iterator& Map<Key, Value>::Iterator::operator++()
	{
		index_++;
		return *this;
	}

	template<typename Key, typename Value> inline typename bool Map<Key, Value>::Iterator::operator!=(const Iterator& other)
	{
		return index_ != other.index_;
	}

	template<typename Key, typename Value> inline typename Map<Key, Value>::Iterator Map<Key, Value>::begin()
	{
		return Iterator(pkeys_, pvalues_, 0);
	}

	template<typename Key, typename Value> inline typename Map<Key, Value>::Iterator Map<Key, Value>::end()
	{
		return Iterator(pkeys_, pvalues_, size_);
	}

	template<typename Key, typename Value> inline size_t Map<Key, Value>::size()
	{
		return size_;
	}

	template<typename Key, typename Value> inline bool Map<Key, Value>::empty()
	{
		return size_ > 0;
	}

	// Eviter de l'utiliser pour une classe sans constructeur par défaut
	template<typename Key, typename Value> inline Value& Map<Key, Value>::operator[](Key key)
	{
		int index = getIndex(key);

		if (index == -1)
		{
			add(key, Value());
			return pvalues_[size_ - 1];
		}

		return pvalues_[getIndex(key)];
	}

	template<typename Key, typename Value> inline Map<Key, Value>& Map<Key, Value>::operator=(const Map<Key, Value>& other)
	{
		if (this != &other)
		{
			if (pkeys_ != nullptr)
			{
				delete[] pkeys_;
				delete[] pvalues_;
			}

			pkeys_ = new Key[other.capacity_];
			pvalues_ = new Value[other.capacity_];
			size_ = other.size_;
			capacity_ = other.capacity_;

			for (int i = 0; i < size_; i++)
			{
				pkeys_[i] = other.pkeys_[i];
				pvalues_[i] = other.pvalues_[i];
			}
		}

		return *this;
	}

	template<typename Key, typename Value> inline Map<Key, Value>& Map<Key, Value>::operator=(Map<Key, Value>&& other)
	{
		if (this != &other)
		{
			if (pkeys_ != nullptr)
			{
				delete[] pkeys_;
				delete[] pvalues_;
			}

			size_ = other.size_;
			capacity_ = other.capacity_;
			pkeys_ = other.pkeys_;
			pvalues_ = other.pvalues_;

			other.pkeys_ = nullptr;
			other.pvalues_ = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
		}

		return *this;
	}
}