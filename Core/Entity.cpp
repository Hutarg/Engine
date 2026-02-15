#include "Entity.h"

namespace blueberry
{
	TypeList<uint32_t> Entity::freeIndices_ = {};
	TypeList<uint32_t> Entity::generations_ = {};

	Map<std::type_index, uint32_t> Entity::types_ = {};
	TypeList<uint32_t> Entity::scripts_ = {};

	TypeList<TypeList<void*>> Entity::components_ = {};
	TypeList<void(*)(void*)> Entity::componentDestructors_ = {};

	Sprite::Sprite(Window window, Pipeline pipeline)
	{
	}

	Entity Script::getEntity()
	{
		return Entity(index_, generation_);
	}

	void Entity::destroyComponents()
	{
		for (int i = 0; i < Entity::components_.size(); i++)
		{
			for (int j = 0; j < Entity::components_[i].size(); j++)
			{
				componentDestructors_[i](Entity::components_[i][j]);
				Entity::components_[i][j] = nullptr;
			}
		}
	}

	Entity::Entity(uint32_t index, uint32_t generation)
	{
		this->index_ = index;
		this->generation_ = generation;
	}

	Entity::Entity()
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

	Entity::Entity(const Entity& other)
	{
		index_ = other.index_;
		generation_ = generation_;
	}

	Entity::Entity(Entity&& other) noexcept
	{
		index_ = other.index_;
		generation_ = other.generation_;
	}

	Entity::~Entity()
	{
		destroy();
	}

	bool Entity::isDestroyed() const
	{
		if (index_ == -1) return false;
		if (generations_[index_] != generation_) return false;
		return true;
	}

	void Entity::destroy() const
	{
		if (index_ != -1)
		{
			freeIndices_.add(index_);
			generations_[index_]++;
		}
	}
}