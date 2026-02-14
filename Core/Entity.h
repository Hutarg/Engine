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

		// Un sprite ne pourra être dessiner que sur une fenêtre à la fois (au pire dupliquez le sprite)
		uint32_t windowIndex_;
		uint32_t windowGeneration_;

		// Les données des sprites seront transmise directement au gpu et aux shaders pour éviter trop de calcul au niveau du cpu 
		// je met ça pour que le shader sache si il faut afficher l'entité
		uint32_t pipelineIndex_;
		uint32_t pipelineGeneration_;

		// Une entité peut ne pas avoir de texture donc il faut que textureIndex puisse être négatif
		// L'index peut représenter une texture mais aussi une animation. Ils seront normalement stockées de manière assez similaire.
		int textureIndex_;
		int textureGeneration_;

	public:

		Sprite(Window window, Pipeline pipeline);

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

		static TypeList<uint32_t> freeIndices_;	// indices libres pour éviter un overflow
		static TypeList<uint32_t> generations_;	// pour s'assurer qu'une entité n'a pas été détruite pour éviter une erreur silencieuse.

		static Map<std::type_index, uint32_t> types_;
		static TypeList<uint32_t> scripts_;
 
		static TypeList<TypeList<void*>> components_;

		template<typename T> static uint32_t getComponentTypeID();

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		Entity(uint32_t index, uint32_t generation);

		friend class Application;
		friend class Script;

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
		if (std::is_base_of<Script, T>::value) scripts_.add(components_.size() - 1);
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

		if (std::is_base_of<Script, T>::value)
		{
			((Script*)component)->index_ = index_;
			((Script*)component)->generation_ = generation_;
		}

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

		if (std::is_base_of<Script, T>::value)
		{
			((Script*)component)->index_ = index_;
			((Script*)component)->generation_ = generation_;
		}

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