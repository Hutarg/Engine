#include "Widget2D.h"

namespace blueberry
{
	TypeList<uint32_t> Widget2D::freeIndices_ = {};
	TypeList<uint32_t> Widget2D::generations_ = {};

	Widget2D::Widget2D(Widget2D* widget_T)
	{
		if (freeIndices_.empty())
		{
			index_ = static_cast<uint32_t>(generations_.size());

			generations_.add(0);
		}
		else
		{
			index_ = freeIndices_[0];
			freeIndices_.remove();

			generations_[index_]++;
		}

		generation_ = generations_[index_];
	}

	Widget2D::Widget2D(uint32_t index, uint32_t generation)
		: index_(index), generation_(generation)
	{
	}
}