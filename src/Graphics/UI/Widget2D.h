#pragma once

#include "../../api.h"

#include "../../Maths/Vector.h"

#include "../Texture.h"

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

		static TypeList<Widget2D*> widgets_;

		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		Widget2D(Widget2D* widget_T);
		Widget2D(uint32_t index, uint32_t generation);

		virtual void update(float dt) = 0;

	public:

		virtual WidgetState getState() = 0;

	};
}