#pragma once

namespace blueberry
{
	template<typename... Args> class Tuple
	{
	private:

		void** pdata_;

		template<size_t I, typename T, typename... Rest> bool compare(void** lhs, void** rhs) const
		{
			if (*(T*)lhs[I] != *(T*)rhs[I]) return false;
			if constexpr (sizeof...(Rest) > 0) return compare<I + 1, Rest...>(lhs, rhs);
			return true;
		}

	public:

		Tuple();
		Tuple(Args... args);
		Tuple(void** pdata);
		Tuple(const Tuple& other);
		Tuple(Tuple&& other);
		~Tuple();

		template<typename T> T& get(int index) const;
		template<typename T> void set(T value, int index);

		bool operator==(Tuple<Args...> other);
		Tuple<Args...>& operator=(Tuple<Args...> other);

	};

	template<typename ...Args> inline Tuple<Args...>::Tuple()
	{
		pdata_ = nullptr;
	}

	template<typename... Args> inline Tuple<Args...>::Tuple(Args... args)
	{
		pdata_ = new void* [sizeof...(Args)];

		int i = 0;
		((pdata_[i++] = new Args(args)), ...);
	}

	template<typename ...Args> inline Tuple<Args...>::Tuple(void** pdata)
	{
		pdata_ = pdata;
	}

	template<typename ...Args> inline Tuple<Args...>::Tuple(const Tuple& other)
	{
		pdata_ = new void* [sizeof...(Args)];

		int i = 0;
		((pdata_[i++] = static_cast<Args*>(other.pdata_[i])), ...);
	}

	template<typename ...Args> inline Tuple<Args...>::Tuple(Tuple&& other)
	{
		pdata_ = other.pdata_;
		other.pdata_ = nullptr;
	}

	template<typename ...Args> inline Tuple<Args...>::~Tuple()
	{
		if (pdata_ != nullptr)
		{
			delete[] pdata_;
		}
	}

	template<typename ...Args> inline bool Tuple<Args...>::operator==(Tuple<Args...> other)
	{
		return compare<0, Args...>(pdata_, other.pdata_);
	}

	template<typename ...Args> template<typename T> inline T& Tuple<Args...>::get(int index) const
	{
		return *static_cast<T*>(pdata_[index]);
	}

	template<typename ...Args> template<typename T> inline void Tuple<Args...>::set(T value, int index)
	{
		pdata_[index] = new T(value);
	}

	template<typename ...Args> inline Tuple<Args...>& Tuple<Args...>::operator=(Tuple<Args...> other)
	{
		void* pdata = pdata_;
		pdata_ = other.pdata_;
		other.pdata_ = pdata;

		return *this;
	}
}