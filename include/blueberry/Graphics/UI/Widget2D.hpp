#pragma once

#include "Export.hpp"

#include "Maths/Vector.hpp"

#include "Graphics/Texture.hpp"

namespace blueberry
{
	enum WidgetState
	{
		NORMAL,
		HOVERED,
		PRESSED,
		DISABLED,
		FOCUSED
	};

	class BLUEBERRY_API Widget2D
	{
	protected:

		static Shrub<Widget2D*> widgets_;

		static Shrub<uint32_t> freeIndices_;
		static Shrub<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		Widget2D(Widget2D* widget_T);
		Widget2D(uint32_t index, uint32_t generation);

		virtual void update(float dt) = 0;

	public:

		virtual WidgetState getState() = 0;

	};
}