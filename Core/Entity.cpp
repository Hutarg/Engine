#include "Entity.h"

#include <chrono>

#include "../Graphics/Animation.h"
#include "../Private/Structs.h"

namespace blueberry
{
	Map<std::type_index, void*> Entity::componentPools = {};
	Map<std::type_index, Entity::ComponentPool<Script*>> Entity::scriptPools = {};

	TypeList<uint32_t> Entity::freeIndices_ = {};
	TypeList<uint32_t> Entity::generations_ = {};

	Sprite::Sprite(Window window, Pipeline pipeline)
	{
		windowIndex_ = window.index_;
		windowGeneration_ = window.generation_;

		pipelineIndex_ = pipeline.index_;
		pipelineGeneration_ = pipeline.generation_;
	}

	Sprite::Sprite(Window window, Pipeline pipeline, Texture texture)
	{
		windowIndex_ = window.index_;
		windowGeneration_ = window.generation_;

		pipelineIndex_ = pipeline.index_;
		pipelineGeneration_ = pipeline.generation_;

		textureIndex_ = texture.index_;
		textureGeneration_ = texture.generation_;

		uvs_[0] = Vector3(1, 1, 0);
		uvs_[1] = Vector3(0, 1, 0);
		uvs_[2] = Vector3(0, 0, 0);
		uvs_[3] = Vector3(1, 0, 0);
	}

	Sprite::Sprite(Window window, Pipeline pipeline, Animation animation)
	{
		windowIndex_ = window.index_;
		windowGeneration_ = window.generation_;

		pipelineIndex_ = pipeline.index_;
		pipelineGeneration_ = pipeline.generation_;

		textureIndex_ = animation.index_;
		textureGeneration_ = animation.generation_;

		Vector4 uv = Texture::textures_[animation.index_].getUV(0);
		float x = uv.getX();
		float mx = x + uv.getZ();
		float y = uv.getY();
		float my = y + uv.getW();

		uvs_[0] = { mx, my, 0 };
		uvs_[1] = { x,	my, 0 };
		uvs_[2] = { x,  y,  0 };
		uvs_[3] = { mx, y,  0 };
	}

	Entity Script::getEntity()
	{
		return Entity(index_, generation_);
	}

	Entity::Entity(uint32_t index, uint32_t generation)
	{
		index_ = index;
		generation_ = generation;
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
		generation_ = other.generation_;
	}

	Entity::Entity(Entity&& other) noexcept
	{
		index_ = other.index_;
		generation_ = other.generation_;
	}

	bool Entity::isAlive() const
	{
		if (index_ == -1) return false;
		if (generations_[index_] != generation_) return false;
		return true;
	}

	void Entity::kill()
	{

	}
}