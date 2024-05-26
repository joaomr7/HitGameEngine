#pragma once

#include "ShaderParser.h"

#include <vector>
#include <string>

namespace hit
{
	ShaderProgram compile_shader_source(const ShaderSource& source);

	int shader_compiler_entry_point(const std::vector<std::string>& options);
}