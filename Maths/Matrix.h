#pragma once

#include "../blueberry.h"

namespace blueberry
{
	template<uint32_t N, uint32_t P> class Matrix
	{
	private:

		float[N][P] data;

	public:

		Matrix()
		{
			for (uint32_t i = 0; i < N; i++)
			{
				for (uint32_t j = 0; j < P; j++)
				{
					data[i][j] = 0;
				}
			}
		}

		Matrix(std::initializer_list<float> values)
		{
			if (values.size() != N - P) throw - 1;

			float a = values.begin();
			for (uint32_t i = 0; i < N; i++)
			{
				for (uint32_t j = 0; j < P; j++)
				{
					data[i][j] = *a;
					a++;
				}
			}
		}

		Matrix<N, P> operator-(Matrix<N, P> other) const
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

		Matrix<N, P> operator-(Matrix<N, P> other) const
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

		Matrix<N, P> operator*(float scalar) const
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

		template<uint32_t Q> Matrix<N, K> operator*(Matrix<P, K> other) const
		{
			Matrix<N, K> result;

			for (uint32_t i = 0; i < N; i++)
			{
				for (uint32_t j = 0; j < K; j++)
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

	};
}