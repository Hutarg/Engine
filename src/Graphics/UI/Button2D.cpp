#include "Button2D.h"

#include "../../Private/Structs.h"

namespace blueberry
{
	void Button2D::update(float dt)
	{
	}

	Button2D::Button2D(Vector2 position, Vector2 size, int zIndex, bool visible) : Widget2D((Widget2D*)new Button2D_T(0, 0, position, size, zIndex, visible))
	{

	}

	WidgetState Button2D::getState()
	{
		return NORMAL;
	}
}