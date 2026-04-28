#pragma once

#include "../../api.h"

#include "Widget2D.h"

#include "../../Utils/TypeList.h"

namespace blueberry
{
	class BLUEBERRY_API Panel2D : public Widget2D
	{
	private:

		struct Panel2D_T;

		void update(float dt) override;

	public:

		Panel2D(Vector2 position, Vector2 size, int zIndex, bool visible);

	};
}