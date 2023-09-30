#pragma once

#include "Log.h"

#include <assert.h>

#ifdef HIT__RELEASE
#define hit_assert(expression, message, ...)
#else
#define hit_assert(expression, message, ...) { if(!(expression)) { hit_fatal(message, __VA_ARGS__); assert(0); } }
#endif