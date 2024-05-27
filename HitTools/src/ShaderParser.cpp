#include "Shader/ShaderParser.h"

#include "Core/Log.h"
#include "Core/Assert.h"
#include "Utils/StringHelpers.h"

#include <sstream>
#include <string_view>

namespace hit::helper
{
	static ShaderData str_to_shader_data(std::string_view str)
	{
		ShaderData data;

		if(str_insensitive_compare(str, "bool"))
		{
			data.type = ShaderData::Bool;
		}
		else if(str_insensitive_compare(str, "int"))
		{
			data.type = ShaderData::Int;
		}

		else if(str_insensitive_compare(str, "ivec2"))
		{
			data.type = ShaderData::Int2;
		}
		else if(str_insensitive_compare(str, "ivec3"))
		{
			data.type = ShaderData::Int3;
		}
		else if(str_insensitive_compare(str, "ivec4"))
		{
			data.type = ShaderData::Int4;
		}
		else if(str_insensitive_compare(str, "float"))
		{
			data.type = ShaderData::Float;
		}
		else if(str_insensitive_compare(str, "vec2"))
		{
			data.type = ShaderData::Float2;
		}
		else if(str_insensitive_compare(str, "vec3"))
		{
			data.type = ShaderData::Float3;
		}
		else if(str_insensitive_compare(str, "vec4"))
		{
			data.type = ShaderData::Float4;
		}
		else if(str_insensitive_compare(str, "mat2"))
		{
			data.type = ShaderData::Mat2;
		}
		else if(str_insensitive_compare(str, "mat3"))
		{
			data.type = ShaderData::Mat3;
		}
		else if(str_insensitive_compare(str, "mat4"))
		{
			data.type = ShaderData::Mat4;
		}

		return data;
	}

}

namespace hit
{
	ShaderParser::ShaderParser(const std::string& source) : m_config(source) { }

	bool ShaderParser::has_configuration()
	{
		// check if it has some configuration
		if(!m_config.has_configuration()) return false;

		// check if it has shader block
		if(!m_config.has_block("Shader")) return false;

		// check if it has name and pass name
		auto shader_block = m_config.get_block("Shader");

		if(shader_block->has_property("name"))
		{
			auto name_property = shader_block->get_property("name");

			if(!name_property.is_single() || !name_property.get_single_value().is_string())
				return false;
		}
		else
		{
			return false;
		}

		if(shader_block->has_property("pass"))
		{
			auto pass_property = shader_block->get_property("pass");

			if (!pass_property.is_single() || !pass_property.get_single_value().is_string())
				return false;
		}
		else
		{
			return false;
		}

		// check if has some type of shader code
		if(!shader_block->has_inner_block("Vertex") && !shader_block->has_inner_block("Fragment")) return false;

		if(shader_block->has_inner_block("Vertex") && !shader_block->get_inner_block("Vertex")->is_source_block())
		{
			return false;
		}

		if(shader_block->has_inner_block("Fragment") && !shader_block->get_inner_block("Fragment")->is_source_block())
		{
			return false;
		}

		return true;
	}

	std::string ShaderParser::get_shader_name()
	{
		return m_config.get_block("Shader")->get_property("name").get_single_value().get_string();
	}

	std::string ShaderParser::get_pass_name()
	{
		return m_config.get_block("Shader")->get_property("pass").get_single_value().get_string();
	}

	bool ShaderParser::has_shader_type(ShaderProgram::Type type)
	{
		auto shader_block = m_config.get_block("Shader");

		if(type == ShaderProgram::Vertex)
		{
			if(!shader_block->has_inner_block("Vertex")) return false;

			return shader_block->get_inner_block("Vertex")->is_source_block();
		}
		else if(type == ShaderProgram::Fragment)
		{
			if(!shader_block->has_inner_block("Fragment")) return false;

			return shader_block->get_inner_block("Fragment")->is_source_block();
		}

		return false;
	}

	ShaderSource ShaderParser::read_shader(ShaderProgram::Type type)
	{
		ShaderSource source;
		source.program.max_uniforms = 0;

		auto shader = m_config.get_block("Shader");

		if(type == ShaderProgram::Vertex)
		{
			source.program.type = type;

			if(shader->has_inner_block("VertexInput"))
			{
				std::vector<std::pair<ShaderData, std::string>> attributes_layout;
				auto attributes = shader->get_inner_block("VertexInput");

				for(const auto& [identifier, property] : attributes->get_properties())
				{
					if (property.is_single() && property.get_single_value().is_string())
					{
						auto data = helper::str_to_shader_data(property.get_single_value().get_string());
						attributes_layout.push_back({ data, identifier });
					}
					else
					{
						hit_error("Property '{}' at 'VertexInput' must be string.", identifier);
					}
				}

				source.program.attributes = BufferLayout(attributes_layout);
			}

			if(shader->has_inner_block("VertexPushConstant"))
			{
				ShaderUniform push_constant;
				push_constant.type = ShaderUniform::PushConstant;
				push_constant.name = "VertexPushConstant";

				std::vector<std::pair<ShaderData, std::string>> layout;
				auto uniform = shader->get_inner_block("VertexPushConstant");

				for(const auto& [identifier, property] : uniform->get_properties())
				{
					if (property.is_single() && property.get_single_value().is_string())
					{
						auto data = helper::str_to_shader_data(property.get_single_value().get_string());
						layout.push_back({ data, identifier });
					}
					else
					{
						hit_error("Property '{}' at block 'VertexPushConstant' must be string.", identifier);
					}
				}
				
				push_constant.layout = BufferLayout(layout);

				source.program.uniforms.push_back(push_constant);
			}

			if(shader->has_inner_block("VertexUniform"))
			{
				std::vector<std::pair<ShaderData, std::string>> layout;
				auto uniform = shader->get_inner_block("VertexUniform");

				if (uniform->has_property("instances"))
				{
					auto property = uniform->get_property("instances");

					if (property.is_single() && property.get_single_value().is_number())
					{
						source.program.max_uniforms = (ui64)std::max(0, (i32)property.get_single_value().get_number());
					}
					else
					{
						hit_warning("VertexUniform 'instances' is on an invalid format. Format must be number. Assuming 1.");
						source.program.max_uniforms = 1;
					}
				}

				if (uniform->has_inner_block("Layout"))
				{
					auto layout_block = uniform->get_inner_block("Layout");

					for (const auto& [identifier, property] : layout_block->get_properties())
					{
						if (property.is_single() && property.get_single_value().is_string())
						{
							auto data = helper::str_to_shader_data(property.get_single_value().get_string());
							layout.push_back({ data, identifier });
						}
					}

					ShaderUniform vertex_uniform;
					vertex_uniform.type = ShaderUniform::UniformBuffer;
					vertex_uniform.name = "VertexUniform";
					vertex_uniform.layout = BufferLayout(layout);

					source.program.uniforms.push_back(vertex_uniform);
				}
			}
			
			auto vertex_source = shader->get_inner_block("Vertex");

			hit_assert(vertex_source, "Vertex source is nullptr!");

			source.source = vertex_source->get_source_block_content();
		}
		else if(type == ShaderProgram::Fragment)
		{
			source.program.type = type;
			
			if(shader->has_inner_block("FragmentUniform"))
			{
				std::vector<std::pair<ShaderData, std::string>> layout;
				auto uniform = shader->get_inner_block("FragmentUniform");

				if (uniform->has_property("instances"))
				{
					auto property = uniform->get_property("instances");

					if (property.is_single() && property.get_single_value().is_number())
					{
						source.program.max_uniforms = (ui64)std::max(0, (i32)property.get_single_value().get_number());
					}
					else
					{
						hit_warning("FragmentUniform 'instances' is on an invalid format. Format must be number. Assuming 1.");
						source.program.max_uniforms = 1;
					}

				}

				for (auto& [identifier, block] : uniform->get_blocks())
				{
					if (identifier == "Layout")
					{
						for (const auto& [identifier, property] : block->get_properties())
						{
							if (property.is_single() && property.get_single_value().is_string())
							{
								auto data = helper::str_to_shader_data(property.get_single_value().get_string());
								layout.push_back({ data, identifier });
							}
						}

						ShaderUniform fragment_uniform;
						fragment_uniform.type = ShaderUniform::UniformBuffer;
						fragment_uniform.name = "FragmentUniform";
						fragment_uniform.layout = BufferLayout(layout);

						source.program.uniforms.push_back(fragment_uniform);
					}
					else if (identifier == "TexturesMaps")
					{
						for (const auto& [identifier, property] : block->get_properties())
						{
							if (property.is_single() && property.get_single_value().is_string())
							{
								if (!str_insensitive_compare(property.get_single_value().get_string(), "texture"))
								{
									hit_error("Invalid type in FragmentUniform::TextureMaps: '{}'.", property.get_single_value().get_string());
									break;
								}

								ShaderUniform texture_uniform;
								texture_uniform.name = identifier;
								texture_uniform.type = ShaderUniform::ImageSampler;

								source.program.uniforms.push_back(texture_uniform);
							}
						}
					}
				}
			}

			auto fragment_source = shader->get_inner_block("Fragment");

			hit_assert(fragment_source, "Fragment source is nullptr!");

			source.source = fragment_source->get_source_block_content();
		}

		return source;
	}
}