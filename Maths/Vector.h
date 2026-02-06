#pragma once

#include <cstdint>
#include <initializer_list>

#include "Functions.h"

namespace blueberry
{
	template<uint32_t D> class alignas(16) Vector
	{
	private:

		float coords_[D];

		Vector(float* coords)
		{
			this->coords_ = coords;
		}

	public:

		Vector()
		{
			coords_ = new float[D];
			
			for (int i = 0; i < D; i++)
			{
				coords_[i] = 0;
			}
		}

		template<typename... Args> Vector(Args... args)
		{
			coords_ = new float[D];

			int k = 0;
			if (sizeof...(args) > 0)
			{
				float temp[] = { static_cast<float>(args)... };

				for (int i = 0; i < sizeof...(args); i++)
				{
					coords_[i] = temp[i];
					k = i;
				}
			}

			for (int j = k; j < D; j++) coords_[j] = 0;
		}

		~Vector()
		{
			delete[] coords_;
		}

		float& operator[](uint32_t index) { return coords_[index]; }

		Vector<D> operator+(const Vector<D>& other)
		{
			float* coords = new float[D];

			for (int i = 0; i < D; i++)
			{
				coords[i] = this->coords_[i] + other.coords_[i];
			}

			return Vector<D>(coords);
		}

		Vector<D> operator+(Vector<D>&& other)
		{
			float* coords = new float[D];

			for (int i = 0; i < D; i++)
			{
				coords[i] = this->coords_[i] + other.coords_[i];
			}

			return Vector<D>(coords);
		}

		float operator*(const Vector<D>& other)
		{
			float scalar = 0;

			for (int i = 0; i < D; i++)
			{
				scalar += this->coords_[i] * other.coords_[i];
			}

			return scalar;
		}

		float operator*(Vector<D>&& other)
		{
			float scalar = 0;

			for (int i = 0; i < D; i++)
			{
				scalar += this->coords_[i] * other.coords_[i];
			}

			return scalar;
		} 
	};

	template<> class alignas(16) Vector<2>
	{
	private:

		float x, y;
		

	public:

		Vector()
		{
			this->x = 0;
			this->y = 0;
		}

		Vector(float x, float y) 
		{
			this->x = x;
			this->y = y;
		}

		float getX() { return x; }
		float getY() { return y; }
		float getLength() { return sqrt(x * x + y * y); }

		Vector<2> operator+(const Vector<2>& other)
		{
			return Vector<2>(x + other.x, y + other.y);
		}

		Vector<2> operator+(Vector<2>&& other)
		{
			return Vector<2>(x + other.x, y + other.y);
		}

		float operator*(const Vector<2>& other)
		{
			return x * other.x + y * other.y;
		}

		float operator*(Vector<2>&& other)
		{
			return x * other.x + y * other.y;
		}
	};

	template<> class alignas(16) Vector<3>
	{
	private:

		float x, y, z;

	public:

		Vector()
		{
			x = 0;
			y = 0;
			z = 0;
		}

		Vector(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		float getX() { return x; }
		float getY() { return y; }
		float getZ() { return z; }

		Vector<3> operator+(const Vector<3>& other)
		{
			return Vector<3>(x + other.x, y + other.y, z + other.z);
		}

		Vector<3> operator+(Vector<3>&& other)
		{
			return Vector<3>(x + other.x, y + other.y, z + other.z);
		}

		float operator*(const Vector<3>& other)
		{
			return x * other.x + y * other.y + z * other.z;
		}

		float operator*(Vector<3>&& other)
		{
			return x * other.x + y * other.y + z * other.z;
		}
	};

	template<> class alignas(16) Vector<4>
	{
	private:

		float x, y, z, w;

	public:

		Vector()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 0;
		}

		Vector(float x, float y, float z, float w) 
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		float getX() { return x; }
		float getY() { return y; }
		float getZ() { return z; }
		float getW() { return w; }

		Vector<4> operator+(const Vector<4>& other)
		{
			return Vector<4>(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		Vector<4> operator+(Vector<4>&& other)
		{
			return Vector<4>(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		float operator*(const Vector<4>& other)
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		float operator*(Vector<4>&& other)
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}
	};

	using Vector2 = Vector<2>;
	using Vector3 = Vector<3>;
	using Vector4 = Vector<4>;
}