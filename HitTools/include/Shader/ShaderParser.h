#pragma once

#include "Core/Types.h"
#include "File/StandardConfigurationFile.h"
#include "Renderer/ShaderTypes.h"

#include <string>

namespace hit
{
	struct ShaderSource
	{
		ShaderProgram program;
		std::string source;
	};

	class ShaderParser
	{
	public:
		ShaderParser(const std::string& source);

		bool has_configuration();

		std::string get_shader_name();
		std::string get_pass_name();

		bool has_shader_type(ShaderProgram::Type type);
		ShaderSource read_shader(ShaderProgram::Type type);

	private:
		config::StandardConfigurationFile m_config;
	};
}