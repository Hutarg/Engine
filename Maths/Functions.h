#pragma once

namespace blueberry
{
	template<typename T, typename D> auto max(T a, D b)
	{
		return (a > b) ? a : b;
	}

	template<typename T, typename D> auto min(T a, D b)
	{
		return (a < b) ? a : b;
	}

	template<typename T, typename D, typename P> auto clamp(T min, D max, P value)
	{
		return (value > max) ? max : ((value < min) ? min : value);
	}

	inline float sqrt(float x)
	{
		float y = x > 1.0f ? x : 1.0f;

		for (int i = 0; i < 5; ++i) y = 0.5f * (y + x / y);

		return y;
	}

	inline float powI(float a, int b)
	{
		float result = 1.0f;
		while (b > 0)
		{
			if (b & 1) result *= a;
			a *= a;
			b >>= 1;
		}
		return result;
	}

	inline float exp(float x)
	{
		const float ln2 = 0.69314718056f;
		const float inv_ln2 = 1.44269504089f;

		int k = (int)(x * inv_ln2);
		float r = x - k * ln2;

		float r2 = r * r;
		float r3 = r2 * r;
		float r4 = r3 * r;

		float exp_r =
			1.f +
			r +
			r2 * 0.5f +
			r3 * (1.f / 6.f) +
			r4 * (1.f / 24.f);

		return exp_r * powI(2, k);
	}

	inline float ln(float x)
	{
		const float ln2 = 0.69314718056f;

		float m;
		int k;
		union {
			float f;
			uint32_t i;
		} u;

		u.f = x;
		k = ((u.i >> 23) & 0xFF) - 127;
		u.i = (u.i & 0x007FFFFF) | 0x3F800000;
		m = u.f;

		float y = (m - 1.f) / (m + 1.f);
		float y2 = y * y;

		float ln_m =
			2.f * (
				y +
				y2 * y / 3.f +
				y2 * y2 * y / 5.f
				);

		return ln_m + k * ln2;
	}

	inline float pow(float a, float b)
	{
		return exp(ln(a) * b);
	}
}