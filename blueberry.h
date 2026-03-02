#pragma once

#ifdef BLUEBERRY_EXPORTS
	#define BLUEBERRY_API __declspec(dllexport)
#else
	#define BLUEBERRY_API __declspec(dllimport)
#endif

#include <cstdint>

#include "Core/Application.h"
#include "Core/Entity.h"

#include "File/File.h"

#include "Graphics/Window.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Texture.h"

#include "Maths/Functions.h"
//#include "Maths/Matrix.h"
#include "Maths/Vector.h"

#include "Utils/Functions.h"
#include "Utils/Image.h"
#include "Utils/Map.h"
#include "Utils/String.h"
#include "Utils/Tuple.h"
#include "Utils/TypeList.h"