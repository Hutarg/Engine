#pragma once

#include "../blueberry.h"

#include "../Utils/TypeList.h"

namespace blueberry
{
	class BLUEBERRY_API Application
	{
	private:

		struct Instance_T;
		struct PhysicalDevice_T;
		struct LogicalDevice_T;
		struct Engine_T;

		static Instance_T instance_;
		static PhysicalDevice_T physicalDevice_;
		static LogicalDevice_T logicalDevice_;
		static Engine_T engine_;

		static bool isRunning_;
		static uint32_t maxFramesInFlight_; // Nombre max de frame en décalée entre le gpu et le cpu

		static void createInstance(const char* appName, int appMajorVersion, int appMinorVersion, int appPatchVersion);
		static void destroyInstance();

		static TypeList<PhysicalDevice_T> getPhysicalDevices();

		static void createLogicalDevice();
		static void destroyLogicalDevice();

		static void createEngine();
		static void destroyEngine();

		static void drawSprites();

		friend class Window;
		friend class Shader;
		friend class Pipeline;

	public:

		static void init();
		static void init(const char* appName, int appMajorVersion, int appMinorVersion, int appPatchVersion);
		static void terminate();

		static void run();
		static void stop();

	};
}