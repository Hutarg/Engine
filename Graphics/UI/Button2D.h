#pragma once

#include "../../api.h"


#include "../../Utils/Map.h"

#include "Widget2D.h"

namespace blueberry
{
	class BLUEBERRY_API Button2D : public Widget2D
	{
	private:

		struct Button2D_T;

		void update(float dt) override;

	public:

		Button2D(Vector2 position, Vector2 size, int zIndex, bool visible);

	};
}