#pragma once

#ifdef BLUEBERRY_EXPORTS
	#define BLUEBERRY_API __declspec(dllexport)
#else
	#define BLUEBERRY_API __declspec(dllimport)
#endif

#include <cstdint>

#include "Core/Application.h"
#include "Core/Entity.h"

#include "Maths/Vector.h"

#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"