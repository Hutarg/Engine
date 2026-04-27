#pragma once

#include "../api.h"

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
		Sprite(Window window);
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
			std::type_index type = std::type_index(typeid(T));

			if (!componentPools.getKeys().contains(type))
			{
				ComponentPool<T>* pool = new ComponentPool<T>();
				componentPools[type] = pool;
				return *pool;
			}

			return *static_cast<ComponentPool<T>*>(componentPools[type]);
		}

		static Map<std::type_index, void*> componentPools;
		static Map<std::type_index, ComponentPool<Script*>> scriptPools;

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

		bool isAlive() const;
		void kill();

		template<typename T> T& setComponent(T component);
		template<typename T, typename... Args> T& setComponent(Args&&... args);
		template<typename T> T& getComponent();

		template<typename T> bool hasComponent();

	};

	template<typename T> inline T& Entity::setComponent(T component)
	{
		if constexpr (std::is_base_of_v<Script, T>)
		{
			std::type_index type = std::type_index(typeid(T));

			if (!scriptPools.getKeys().contains(type))
			{
				ComponentPool<Script*> pool = ComponentPool<Script*>();
				scriptPools[type] = pool;
			}

			ComponentPool<Script*>& pool = scriptPools[type];

			if (index_ >= pool.componentsIndices.size()) pool.componentsIndices.resize(index_ + 1, UINT32_MAX);

			Script* script = static_cast<Script*>(new T(component));
			script->index_ = index_;
			script->generation_ = generation_;

			if (hasComponent<T>())
			{
				pool.components[pool.componentsIndices[index_]] = script;
				return *static_cast<T*>(script);
			}

			pool.components.add(script);
			pool.entitiesIndices.add(index_);
			pool.componentsIndices[index_] = pool.components.size() - 1;

			return *static_cast<T*>(script);
		}

		ComponentPool<T>& pool = getComponentPool<T>();

		if (index_ >= pool.componentsIndices.size()) pool.componentsIndices.resize(index_ + 1, -1);

		if (hasComponent<T>())
		{
			pool.components[pool.componentsIndices[index_]] = component;
			return pool.components[pool.componentsIndices[index_]];
		}

		pool.components.add(component);
		pool.entitiesIndices.add(index_);
		pool.componentsIndices[index_] = pool.components.size() - 1;

		return pool.components[pool.componentsIndices[index_]];
	}

	template<typename T, typename ...Args> inline T& Entity::setComponent(Args&&... args)
	{
		return setComponent(T(std::forward<Args>(args)...));
	}

	template<typename T> inline T& Entity::getComponent()
	{
		if constexpr (std::is_base_of_v<Script, T>)
		{
			ComponentPool<Script*>& pool = scriptPools[std::type_index(typeid(T))];
			return pool.components[pool.componentsIndices[index_]];
		}

		ComponentPool<T>& pool = getComponentPool<T>();
		return pool.components[pool.componentsIndices[index_]];
	}

	template<typename T> inline bool Entity::hasComponent()
	{
		if constexpr (std::is_base_of_v<Script, T>)
		{
			std::type_index type = std::type_index(typeid(T));
			if (!scriptPools.getKeys().contains(type)) return false;

			ComponentPool<Script*>& pool = scriptPools[type];

			if (index_ >= pool.componentsIndices.size()) return false;

			uint32_t componentIndex = pool.componentsIndices[index_];

			return componentIndex < pool.components.size() && pool.entitiesIndices[componentIndex] == index_;
		}

		ComponentPool<T>& pool = getComponentPool<T>();

		if (index_ >= pool.componentsIndices.size()) return false;

		uint32_t componentIndex = pool.componentsIndices[index_];
		return componentIndex < pool.components.size() and pool.entitiesIndices[componentIndex] == index_;
	}
}