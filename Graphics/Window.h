#pragma once

#include "../blueberry.h"

#include <cstdint>

#include "../Maths/Vector.h"
#include "../Utils/TypeList.h"

namespace blueberry
{
	class BLUEBERRY_API Window
	{
	private:

		struct Window_T;
		struct WindowInfos_T;

		static TypeList<Window_T> windows_;
		static TypeList<uint32_t> freeIndices_;
		static TypeList<uint32_t> generations_;

		static WindowInfos_T windowInfos_;

		uint32_t index_ = -1;
		uint32_t generation_ = -1;

		friend class Application;
		friend class Pipeline;
		friend class Functions;

	public:

		Window(const char* title, uint32_t width, uint32_t height);

		bool shouldClose() const;
		bool isClosed() const;
		void close() const;

		uint32_t getWidth() const;
		uint32_t getHeight() const;
		Vector2 getSize() const;

	};
}