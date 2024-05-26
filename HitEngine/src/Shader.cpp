#include "Renderer/Shader.h"
#include "File/SerialFile.h"

namespace hit
{
	bool Shader::bind()
	{
		hit_assert(m_pipeline, "Shader pipeline is null! Can't bind.");
		return m_pipeline->bind_pipeline();
	}

	bool Shader::unbind()
	{ 
		hit_assert(m_pipeline, "Shader pipeline is null! Can't unbind.");
		return m_pipeline->unbind_pipeline();
	}

	bool Shader::bind_attribure(const Ref<ShaderAttribute>& attribute)
	{
		hit_assert(attribute, "Shader attribute is null. Can't bind it.");
		hit_assert(m_pipeline, "Shader pipeline is null! Can't bind attribute.");
		return m_pipeline->bind_instance(attribute->m_instance);
	}

	bool Shader::unbind_attribure(const Ref<ShaderAttribute>& attribute)
	{
		hit_assert(attribute, "Shader attribute is null. Can't unbind it.");
		hit_assert(m_pipeline, "Shader pipeline is null! Can't unbind attribute.");
		return m_pipeline->unbind_instance(attribute->m_instance);
	}

	bool Shader::has_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name)
	{
		return m_pipeline->has_uniform_data(at, uniform_name, data_name);
	}

	ui64 Shader::get_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name)
	{
		return m_pipeline->get_uniform_data_location(at, uniform_name, data_name);
	}

	bool Shader::write_data(ShaderAttribute* attribute, ui64 offset, ui64 size, void* data)
	{
		hit_assert(attribute, "Attribute is null!");
		return m_pipeline->update_instance(attribute->m_instance, offset, size, data);
	}

	bool Shader::write_constant(ShaderProgram::Type at, ui64 size, void* data)
	{
		hit_assert(size > 0 && data, "Writing null constant to shader!");
		return m_pipeline->push_constant(at, size, data);
	}

	bool Shader::load_programs(std::string_view programs_path, std::vector<ShaderProgram>& programs)
	{
		SerialFileReader reader(programs_path);

		ui64 programs_count = 0;
		if (!reader.get_deserializer().try_deserialize(&programs_count))
		{
			hit_error("Failed to deserealize shader at '{}'.", programs_path);
			return false;
		}

		if (programs_count == 0)
		{
			hit_error("Can't read 0 shader programs.");
			return false;
		}

		programs.resize(programs_count);
		for (auto& p : programs)
		{
			if (!p.deserialize(reader.get_deserializer()))
			{
				hit_error("Failed to desearilize shader program '{}'.", programs_path);
				return false;
			}
		}

		return true;
	}

	bool Shader::register_attribute(ShaderProgram::Type program_type, const Ref<ShaderAttribute>& attribute)
	{
		hit_assert(attribute, "Shader attribute is null!");

		attribute->m_shader = this;
		attribute->m_instance = m_pipeline->create_instance((ui64)program_type);
		return attribute->m_instance.is_valid();
	}

	void Shader::unregister_attribute(const Ref<ShaderAttribute>& attribute)
	{ 
		hit_assert(attribute, "Shader attribute is null!");
		m_pipeline->destroy_instance(attribute->m_instance);
	}

	bool Shader::create_pipeline(const PipelineConfig& config)
	{
		hit_assert(m_renderer, "Shader renderer instance is null.");
		m_pipeline = m_renderer->acquire_render_pipeline();

		return m_pipeline->create(config);
	}

	void Shader::destroy_pipeline()
	{
		hit_assert(m_pipeline, "Shader pipeline is null! Can't destroy it.");
		m_pipeline->destroy();
		m_pipeline = nullptr;
	}
}