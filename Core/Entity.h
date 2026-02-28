#pragma once

#include "../blueberry.h"

#include <vector>
#include <iostream>
#include <typeindex>
#include <type_traits>

#include "../Graphics/Window.h"

#include "../Maths/Vector.h"
#include "../Maths/Matrix.h"

#include "../Utils/TypeList.h"
#include "../Utils/Map.h"

namespace blueberry
{
	class Entity;
	class Texture;
	class Animation;

	class BLUEBERRY_API alignas(16) Transform
	{
	private:

		Vector4 position;
		Vector4 rotation;
		Vector4 scale;

	public:

		Transform() {}

		Transform(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz)
		{
			position = { x, y, z, 0 };
			rotation = { rx, ry, rz, 0 };
			scale = { sx, sy, sz, 0 };
		}

		Transform(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			this->position = { position.getX(), position.getY(), position.getZ(), 0 };
			this->rotation = { rotation.getX(), rotation.getY(), rotation.getZ(), 0 };
			this->scale = { scale.getX(), scale.getY(), scale.getZ(), 0 };
		}

		Vector3 getPosition()
		{
			return Vector3(position.getX(), position.getY(), position.getZ());
		}

		Vector3 getRotation()
		{
			return Vector3(rotation.getX(), rotation.getY(), rotation.getZ());
		}

		Vector3 getScale()
		{
			return Vector3(scale.getX(), scale.getY(), scale.getZ());
		}
	};

	class alignas(16) BLUEBERRY_API Sprite
	{
	private:

		uint32_t windowIndex_ = -1;
		uint32_t windowGeneration_ = -1;

		uint32_t pipelineIndex_ = -1;
		uint32_t pipelineGeneration_ = -1;

		uint32_t textureIndex_ = -1;
		uint32_t textureGeneration_ = -1;

		Vector3 uvs_[4];

		friend class Application;

	public:

		Sprite() = default;
		Sprite(Window window, Pipeline pipeline);
		Sprite(Window window, Pipeline pipeline, Texture texture);
		Sprite(Window window, Pipeline pipeline, Animation animation);

	};

	class BLUEBERRY_API Script
	{
	private:

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Application;
		friend class Entity;

	protected:

		virtual void create() {};
		virtual void update(float dt) {};
		virtual void destroy() {};

		Entity getEntity();

	};

	class BLUEBERRY_API Entity
	{
	private:

		template<typename T> struct ComponentPool
		{
			TypeList<T> components;
			TypeList<uint32_t> entitiesIndices;			// entitiesIndices[componentIndex] -> EntityIndex
			TypeList<uint32_t> componentsIndices;		// componentsIndices[entityIndex] -> ComponentIndex
		};

		template<typename T> static ComponentPool<T>& getComponentPool()
		{
			static ComponentPool<T> pool;
			return pool;
		}

		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		uint32_t index_;
		uint32_t generation_;

		Entity(uint32_t index, uint32_t generation);
		
		friend class Application;
		friend class Script;

	public:

		Entity();
		Entity(const Entity& other);
		Entity(Entity&& other) noexcept;
		~Entity();

		bool isAlive() const;
		void destroy() const;

		template<typename T> T& setComponent(T component);
		template<typename T, typename... Args> T& setComponent(Args&&... args);
		template<typename T> T& getComponent();

		template<typename T> bool hasComponent();

	};

	template<typename T> inline T& Entity::setComponent(T component)
	{
		ComponentPool<T>& pool = getComponentPool<T>();

		if (hasComponent<T>())
		{
			pool.components[pool.componentsIndices[index_]] = component;
			return pool.components[pool.componentsIndices[index_]];
		}
		
		if (index_ >= pool.entitiesIndices.size()) pool.entitiesIndices.resize(index_ + 1, -1);

		pool.componentsIndices[index_] = pool.components.size();
		pool.entitiesIndices.add(index_);
		pool.components.add(component);
	}

	template<typename T, typename ...Args> inline T& Entity::setComponent(Args&&... args)
	{
		return setComponent(T(std::forward<Args>(args)...));
	}

	template<typename T> inline T& Entity::getComponent()
	{
		ComponentPool<T> pool = getComponentPool<T>();

		if (index_ >= pool.entitiesIndices.size()) throw - 1;
		if (pool.entitiesIndices[index_] >= pool.components.size()) throw - 1;

		return pool.components[pool.entitiesIndices[index_]];
	}

	template<typename T> inline bool Entity::hasComponent()
	{
		ComponentPool<T>& pool = getComponentPool<T>();

		if (index_ >= pool.componentsIndices.size()) return false;

		uint32_t componentIndex = pool.componentsIndices[index_];
		return componentIndex < pool.components.size() and pool.entitiesIndices[componentIndex] == index_;
	}
}