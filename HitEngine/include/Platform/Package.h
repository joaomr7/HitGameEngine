#pragma once

#ifdef HIT_PLATFORM_WINDOWS
#define HIT_PACKAGE_FUN extern "C" __declspec(dllexport)
#else
#define HIT_PACKAGE_FUN
#endif