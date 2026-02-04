#pragma once

#include "../blueberry.h"

#include <vector>
#include <iostream>
#include <typeindex>

#include "../Graphics/Window.h"

#include "../Maths/Vector.h"

#include "../Utils/TypeList.h"
#include "../Utils/Map.h"

namespace blueberry
{
	class BLUEBERRY_API Transform
	{
	private:

		Vector3 position;
		Vector3 rotation;
		Vector3 scale;

	public:

		Transform() {}

		Transform(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz)
		{
			position = { x, y, z };
			rotation = { rx, ry, rz };
			scale = { sx, sy, sz };
		}

		Vector3 getPosition()
		{
			return position;
		}

		Vector3 getRotation()
		{
			return rotation;
		}

		Vector3 getScale()
		{
			return scale;
		}
	};

	class alignas(16) BLUEBERRY_API Sprite
	{
	private:

		uint32_t windowIndex_; // Un sprite ne pourra être diplay que sur une fenêtre à la fois (au pire dupliquez le sprite)
		uint32_t windowGeneration_;
		uint32_t pipelineIndex_;
		uint32_t pipelineGeneration_;
		uint32_t textureIndex_; // 
		uint32_t textureGeneration_;

	public:

		Sprite(Window window, Pipeline pipeline);

	};

	class BLUEBERRY_API Entity
	{
	private:

		static TypeList<uint32_t> freeIndices_;	// indices libres pour éviter un overflow
		static TypeList<uint32_t> generations_;	// pour s'assurer qu'une entité n'a pas été détruite pour éviter une erreur silencieuse.

		static Map<std::type_index, uint32_t> types_;

		static TypeList<TypeList<void*>> components_;

		template<typename T> static uint32_t getComponentTypeID();

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		Entity(uint32_t index, uint32_t generation);

		friend class Application;

	public:

		Entity();
		Entity(const Entity& other);
		Entity(Entity&& other) noexcept;
		~Entity();

		bool isDestroyed() const;
		void destroy() const;

		template<typename T> T& setComponent(T component);
		template<typename T, typename... Args> T& setComponent(Args&&... args);
		template<typename T> T& getComponent();

		template<typename T> bool hasComponent();

	};

	template<typename T> uint32_t Entity::getComponentTypeID()
	{
		int index = types_.getKeys().getIndex(std::type_index(typeid(T))); // changer getIndex pour du log(n)

		if (index != -1) return index;
		
		types_.add(std::type_index(typeid(T)), components_.size());
		components_.add({});
		return components_.size() - 1;
	}

	template<typename T> T& Entity::setComponent(T t)
	{
		uint32_t type = getComponentTypeID<T>();
		if (components_.size() <= type) components_.resize(static_cast<size_t>(type) + 1, {});

		TypeList<void*>& pool = components_[type];
		if (pool.size() <= index_) pool.resize(static_cast<uint64_t>(index_) + 1, {});

		T* component = new T(t);
		pool[index_] = static_cast<void*>(component);

		return *component;
	}

	template<typename T, typename... Args> T& Entity::setComponent(Args&&... args)
	{
		uint32_t type = getComponentTypeID<T>();
		if (components_.size() <= type) components_.resize(static_cast<size_t>(type) + 1, {});

		TypeList<void*>& pool = components_[type];
		if (pool.size() <= index_) pool.resize(static_cast<uint64_t>(index_) + 1, {});

		T* component = new T(std::forward<Args>(args)...);
		pool[index_] = static_cast<void*>(component);

		return *component;
	}

	template<typename T> T& Entity::getComponent()
	{
		return *static_cast<T*>(components_[getComponentTypeID<T>()][index_]);
	}

	template<typename T> bool Entity::hasComponent()
	{
		uint32_t type = getComponentTypeID<T>();

		if (components_.size() <= type) return false;
		if (components_[type].size() <= index_) return false;

		return true;
	}
}