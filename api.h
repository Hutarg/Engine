#pragma once

#ifdef BLUEBERRY_EXPORTS
#define BLUEBERRY_API __declspec(dllexport)
#else
#define BLUEBERRY_API __declspec(dllimport)
#endif