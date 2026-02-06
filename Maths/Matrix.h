#pragma once

#include "../blueberry.h"

namespace blueberry
{
	template<uint32_t N, uint32_t P> class Matrix
	{
	private:

		alignas(16) float data[N][P];

	public:

		Matrix();
		Matrix(std::initializer_list<float> values);
		~Matrix() = default;

		Matrix<N, P> operator+(Matrix<N, P> other) const;
		Matrix<N, P> operator-(Matrix<N, P> other) const;
		Matrix<N, P> operator*(float scalar) const;
		template<uint32_t Q> Matrix<N, Q> operator*(Matrix<P, Q> other) const;

		Vector<P> operator[](uint32_t i) const;
	};

	template<uint32_t N, uint32_t P> Matrix<N, P>::Matrix()
	{
		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < P; j++)
			{
				data[i][j] = 0;
			}
		}
	}

	template<uint32_t N, uint32_t P> Matrix<N, P>::Matrix(std::initializer_list<float> values)
	{
		if (values.size() != N * P) throw - 1;

		const float* a = values.begin();
		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < P; j++)
			{
				data[i][j] = *a;
				a++;
			}
		}
	}
	
	template<uint32_t N, uint32_t P> Matrix<N, P> Matrix<N, P>::operator+(Matrix<N, P> other) const
	{
		Matrix<N, P> r = {};

		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < P; j++)
			{
				r.data[i][j] = data[i][j] + other.data[i][j];
			}
		}

		return r;
	}

	template<uint32_t N, uint32_t P> inline Matrix<N, P> Matrix<N, P>::operator-(Matrix<N, P> other) const
	{
		Matrix<N, P> r = {};

		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < P; j++)
			{
				r.data[i][j] = data[i][j] * other.data[i][j];
			}
		}

		return r;
	}

	template<uint32_t N, uint32_t P> inline Matrix<N, P> Matrix<N, P>::operator*(float scalar) const
	{
		Matrix<N, P> r = {};

		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < P; j++)
			{
				r.data[i][j] = data[i][j] * scalar;
			}
		}

		return r;
	}

	template<uint32_t N, uint32_t P> template<uint32_t Q> inline Matrix<N, Q> Matrix<N, P>::operator*(Matrix<P, Q> other) const
	{
		Matrix<N, Q> result;

		for (uint32_t i = 0; i < N; i++)
		{
			for (uint32_t j = 0; j < Q; j++)
			{
				float sum = 0;

				for (uint32_t k = 0; k < P; ++k)
				{
					sum += data[i][k] * other.data[k][j];
				}

				result.data[i][j] = sum;
			}
		}

		return result;
	}


	template<uint32_t N, uint32_t P> Vector<P> Matrix<N, P>::operator[](uint32_t i) const
	{
		return Vector<P>(data[i]);
	}
}