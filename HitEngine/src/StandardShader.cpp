#include "Renderer/Shaders/StandardShader.h"

namespace hit
{
	bool StandardShader::create(const Ref<Renderpass>& pass)
	{
		PipelineConfig config;
		config.pass = pass;

		// load programs
		if (!load_programs("assets/shaders/compiled/StandardShader.hit_shader", config.programs))
		{
			hit_error("Failed to load shader programs.");
		}

		// setup color blend
		config.color_blend.use_logical = false;
		config.color_blend.use_blend = true;
		config.color_blend.mask = PipelineColorBlending::MaskAll;
		config.color_blend.color_blend_op = PipelineColorBlending::OpAdd;
		config.color_blend.src_color_factor = PipelineColorBlending::FactorSrcAlpha;
		config.color_blend.dest_color_factor = PipelineColorBlending::FactorOneMinusSrcAlpha;
		config.color_blend.alpha_blend_op = PipelineColorBlending::OpAdd;
		config.color_blend.src_alpha_factor = PipelineColorBlending::FactorOne;
		config.color_blend.dest_alpha_factor = PipelineColorBlending::FactorZero;

		// setup rendering
		config.topology = PipelineTopology::TriangleList;
		config.polygon = PipelinePolygon::Fill;
		config.cull_mode = PipelineCullMode::Back;
		config.front_face_clockwise = true;
		config.write_depth = false;
		config.use_depth = false;
		//config.use_depth = true;
		//config.write_depth = true;

		return create_pipeline(config);
	}

	void StandardShader::destroy()
	{ 
		destroy_pipeline();
	}

	Ref<ShaderAttribute> StandardShader::create_program_attribute(ShaderProgram::Type program_type)
	{
		Ref<ShaderAttribute> attribute = nullptr;
		switch (program_type)
		{
			case ShaderProgram::Vertex:
			{
				attribute = create_ref<StandardGlobalData>();
				break;
			}
			case ShaderProgram::Fragment:
			{
				attribute = create_ref<StandardMaterial>();
				break;
			}
		}

		hit_assert(attribute, "Invalid attribute program type!");

		if (!register_attribute(program_type, attribute))
		{
			hit_error("Failed to create attribute shader instance.");
			return nullptr;
		}
		
		if (!attribute->create_resources())
		{
			hit_error("Failed to create shader attribute resources.");

			unregister_attribute(attribute);
			return nullptr;
		}

		return attribute;
	}

	void StandardShader::destroy_program_attribute(const Ref<ShaderAttribute>& attribute)
	{
		hit_assert(attribute, "Shader attribute is null!");
		attribute->release_resources();
		unregister_attribute(attribute);
	}

	bool StandardGlobalData::create_resources()
	{
		auto shader = (Shader*)get_shader();
		hit_assert(shader, "Shader is null!");

		if (!shader->has_data_location(ShaderProgram::Vertex, "VertexUniform", "projection"))
		{
			hit_error("Can't find global_uniform::projection");
			return false;
		}

		if (!shader->has_data_location(ShaderProgram::Vertex, "VertexUniform", "view"))
		{
			hit_error("Can't find vertex_uniform::view");
			return false;
		}

		m_projection_matrix_location = shader->get_data_location(ShaderProgram::Vertex, "VertexUniform", "projection");
		m_view_matrix_location = shader->get_data_location(ShaderProgram::Vertex, "VertexUniform", "view");

		set_view_matrix(m_view_matrix);
		set_projection_matrix(m_projection_matrix);

		return true;
	}

	void StandardGlobalData::release_resources()
	{ }

	void StandardGlobalData::set_view_matrix(const Mat4& view_matrix)
	{
		auto shader = (Shader*)get_shader();
		hit_assert(shader, "Shader is null!");

		m_view_matrix = view_matrix;
		if (!shader->write_data(this, m_view_matrix_location, sizeof(Mat4), m_view_matrix.data))
		{
			hit_error("Failed to write view matrix to shader buffer!");
		}
	}

	void StandardGlobalData::set_projection_matrix(const Mat4& projection_matrix)
	{
		auto shader = (Shader*)get_shader();
		hit_assert(shader, "Shader is null!");

		m_projection_matrix = projection_matrix;
		if (!shader->write_data(this, m_projection_matrix_location, sizeof(Mat4), m_projection_matrix.data))
		{
			hit_error("Failed to write projection matrix to shader buffer!");
		}
	}

	bool StandardMaterial::create_resources()
	{
		auto shader = (Shader*)get_shader();
		hit_assert(shader, "Shader is null!");

		if (!shader->has_data_location(ShaderProgram::Fragment, "instance_uniform", "base_color"))
		{
			hit_error("Can't find instance_uniform::base_color");
			return false;
		}

		m_base_color_location = shader->get_data_location(ShaderProgram::Fragment, "instance_uniform", "base_color");
		set_base_color(m_base_color);

		return true;
	}

	void StandardMaterial::release_resources()
	{ }

	void StandardMaterial::set_base_color(const Vec4& base_color)
	{
		auto shader = (Shader*)get_shader();
		hit_assert(shader, "Shader is null!");

		m_base_color = base_color;
		if (!shader->write_data(this, m_base_color_location, sizeof(Vec4), m_base_color.elements))
		{
			hit_error("Failed to write base color to shader buffer!");
		}
	}
}