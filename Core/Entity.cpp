#include "Entity.h"

static uint32_t fnv1a(const char* str)
{
	uint32_t hash = 2166136261u;
	while (*str)
	{
		hash ^= (uint8_t)*str++;
		hash *= 16777619u;
	}
	return hash;
}

namespace blueberry
{
	TypeList<uint32_t> Entity::freeIndices_ = {};
	TypeList<uint32_t> Entity::generations_ = {};

	TypeList<const char*> Entity::names_ = {};
	TypeList<uint32_t> Entity::hashedNames_ = {};
	TypeList<uint32_t> Entity::sortedNames_ = {};

	TypeList<TypeList<void*>> Entity::components_ = {};

	bool Entity::findHashRange(uint32_t hash, uint32_t& first, uint32_t& last)
	{
		uint32_t n = static_cast<uint32_t>(sortedNames_.size());
		if (n == 0) return false;

		uint32_t left = 0;
		uint32_t right = n;
		while (left < right)
		{
			uint32_t mid = (left + right) / 2;
			if (hashedNames_[sortedNames_[mid]] < hash)
				left = mid + 1;
			else
				right = mid;
		}

		if (left == n || hashedNames_[sortedNames_[left]] != hash) return false;

		first = left;

		right = n;
		while (left < right)
		{
			uint32_t mid = (left + right) / 2;
			if (hashedNames_[sortedNames_[mid]] <= hash)
				left = mid + 1;
			else
				right = mid;
		}

		last = left - 1;
		return true;
	}

	// Pour trouver l'index en log(n)
	uint32_t Entity::getHashIndex(const char* name)
	{
		uint32_t hash = fnv1a(name);

		uint32_t first, last;
		if (!findHashRange(hash, first, last)) return -1;

		for (uint32_t i = first; i <= last; ++i)
		{
			uint32_t index = sortedNames_[i];
			if (strcmp(names_[index], name) == 0) return index;
		}

		return -1;
	}

	Entity::Entity(uint32_t index, uint32_t generation)
	{
		this->index_ = index;
		this->generation_ = generation;
	}

	Entity Entity::getEntity(const char* name)
	{
		uint32_t index = getHashIndex(name);

		if (index == -1)
		{
			std::cerr << "No entity have the name : " << name << "\n";
			return Entity(-1, -1);
		}

		return Entity(index, generations_[index]);
	}

	Entity::Entity(const char* name)
	{
		index_ = getHashIndex(name);
		if (index_ != -1)
		{
			std::cerr << "An entity with the name : \"" << name << "\" already exist. If you want to retrieve an entity by name use getEntity().\n";
			return;
		}

		if (freeIndices_.empty())
		{
			index_ = static_cast<uint32_t>(generations_.size());

			generations_.add(0);
			names_.add(name);
			hashedNames_.add(fnv1a(name));
		}
		else
		{
			index_ = freeIndices_[0];
			freeIndices_.remove();

			generations_[index_]++;
			names_[index_] = name;
			hashedNames_[index_] = fnv1a(name);
		}

		uint32_t left = 0;
		uint32_t right = static_cast<uint32_t>(sortedNames_.size());

		while (left < right)
		{
			uint32_t mid = (left + right) / 2;

			if (hashedNames_[sortedNames_[mid]] < hashedNames_[index_]) left = mid + 1;
			else right = mid;
		}

		sortedNames_.add(index_, left);
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

	const char* Entity::getName() const
	{
		return names_[index_];
	}
}