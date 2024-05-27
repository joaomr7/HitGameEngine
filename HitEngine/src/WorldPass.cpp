#include "Renderer/Passes/WorldPass.h"

#include "File/SerialFile.h"

namespace hit
{
	struct Vertex
	{
		Vec3 position;
		Vec2 uv;
	};

	bool WorldPass::generate_resources(ui32 images_width, ui32 images_height)
	{
		RendergraphResource resource;
		resource.name = WORLD_COLOR_RESOURCE_NAME;
		resource.type = RendergraphResource::TypeColor;
		resource.origin = RendergraphResource::OriginGlobal;

		m_resources.push_back(resource);

		return true;
	}

	bool WorldPass::initialize()
	{
		m_quad = get_renderer()->acquire_buffer();
		m_std_shader = create_ref<StandardShader>((Renderer*)get_renderer());
		m_quad = get_renderer()->acquire_buffer();
		m_quad_indices = get_renderer()->acquire_buffer();

		ui64 quad_size = 4 * sizeof(Vertex);
		ui64 indices_size = 6 * sizeof(ui32);
		if (!m_quad->create(quad_size, BufferType::Vertex, BufferAllocationType::None))
		{
			hit_error("Failed to create quad.");
			return false;
		}

		if (!m_quad_indices->create(indices_size, BufferType::Index, BufferAllocationType::None))
		{
			hit_error("Failed to create quad indices.");
			return false;
		}

		Vertex quad[4] = {
			{ {  0.5f,  0.5f, 1.0f }, { 1.0f, 1.0f } },
			{ {  0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f } },
			{ { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f } }
		};

		ui32 quad_indices[6] = { 0, 1, 3, 1, 2, 3 };

		if (!m_quad->load(0, quad_size, quad))
		{
			hit_error("Failed to load quad.");
			return false;
		}

		if (!m_quad_indices->load(0, indices_size, quad_indices))
		{
			hit_error("Failed to load quad indices.");
			return false;
		}

		if (!m_std_shader->create(get_pass()))
		{
			hit_error("Failed to create standard shader.");
			return false;
		}

		m_global_data = cast_ref<StandardGlobalData>(m_std_shader->create_program_attribute(ShaderProgram::Vertex));
		m_std_material = cast_ref<StandardMaterial>(m_std_shader->create_program_attribute(ShaderProgram::Fragment));

		// load hit logo
		{
			SerialFileReader reader("assets/images/textures/HIT_logo.hit_texture");
			auto& deserializer = reader.get_deserializer();
			
			i32 image_width;
			i32 image_height;
			i32 image_channels;

			if (!deserializer.try_deserialize(&image_width))
			{
				hit_error("Failed to load image width.");
				return false;
			}

			if (!deserializer.try_deserialize(&image_height))
			{
				hit_error("Failed to load image height.");
				return false;
			}

			if (!deserializer.try_deserialize(&image_channels))
			{
				hit_error("Failed to load image channels.");
				return false;
			}

			std::vector<ui8> data(image_width * image_height * image_channels);
			if (!deserializer.try_deserialize_array(data.data(), data.size()))
			{
				hit_error("Failed to load image.");
				return false;
			}

			TextureInfo texture_info;
			texture_info.source = TextureInfo::SourceOwn;
			texture_info.format = TextureInfo::FormatRGBA;
			texture_info.type = TextureInfo::Type2D;

			texture_info.writable = false;
			texture_info.width = image_width;
			texture_info.height = image_height;
			texture_info.channels = image_channels;

			texture_info.use_sampler = true;
			texture_info.sampler_info.min_filter = SamplerInfo::FilterLinear;
			texture_info.sampler_info.mag_filter = SamplerInfo::FilterLinear;
			texture_info.sampler_info.wrapper = SamplerInfo::WrapperRepeat;

			auto hit_texture = get_renderer()->acquire_texture();
			if (!hit_texture->create(texture_info, data.data()))
			{
				hit_error("Failed to create texture.");
				return false;
			}

			hit_texture->set_release_mode(Releseable::ModeAtFirstOpportunity);
			m_std_material->set_base_texture(hit_texture);
		}

	#if 0
		f32 aspect_ratio = 16.f / 9.f;
		f32 ortho_size = 2.f;
		Mat4 projection = mat4_orthographic(
			-ortho_size * aspect_ratio * 0.5f,
			ortho_size * aspect_ratio * 0.5f,
			-ortho_size * 0.5f,
			ortho_size * 0.5f,
			0.001f, 1000.f
		);
	#else
		f32 aspect_ratio = 16.f / 9.f;
		Mat4 projection = mat4_perspective(80.f, aspect_ratio, 0.0001f, 10000.f);
	#endif

		m_global_data->set_projection_matrix(projection);
		m_global_data->set_view_matrix(mat4_identity());

		return true;
	}

	void WorldPass::shutdown()
	{ 
		m_std_shader->destroy_program_attribute(m_global_data);
		m_std_shader->destroy_program_attribute(m_std_material);
		m_std_shader->destroy();
		m_quad->destroy();
		m_quad_indices->destroy();

		m_quad = nullptr;
		m_quad_indices = nullptr;
		m_std_shader = nullptr;
		m_global_data = nullptr;
		m_std_material = nullptr;
	}

	void WorldPass::on_render(FrameData* frame_data)
	{
		if (m_std_shader->bind())
		{
			if (!m_std_shader->write_constant(ShaderProgram::Vertex, sizeof(Mat4), &m_model))
			{
				hit_error("Failed to write constant.");
			}

			if (!m_std_shader->bind_attribure(m_global_data))
			{
				hit_error("Failed to bind global data.");
			}

			if (!m_std_shader->bind_attribure(m_std_material))
			{
				hit_error("Failed to bind material.");
			}

			m_quad->bind();
			m_quad->draw(0, 4, true);

			m_quad_indices->bind();
			m_quad_indices->draw(0, 6, false);

			m_quad->unbind();
			m_quad_indices->unbind();

			m_std_shader->unbind();
		}
		else
		{
			hit_error("Failed to bind shader.");
		}
	}

	bool WorldPass::on_resize(ui32 new_width, ui32 new_height)
	{

	#if 0
		f32 aspect_ratio = (f32)new_width / (f32)new_height;
		f32 ortho_size = 2.f;
		Mat4 projection = mat4_orthographic(
			-ortho_size * aspect_ratio * 0.5f,
			ortho_size * aspect_ratio * 0.5f,
			-ortho_size * 0.5f,
			ortho_size * 0.5f,
			0.001f, 1000.f
		);
	#else
		f32 aspect_ratio = (f32)new_width / (f32)new_height;
		Mat4 projection = mat4_perspective(80.f, aspect_ratio, 0.0001f, 10000.f);
	#endif

		m_global_data->set_projection_matrix(projection);
		return true;
	}

	UnbakedPass world_pass_create_builtin_pass(ui32 render_area_width, ui32 render_area_height)
	{
		UnbakedPass world_pass;
		world_pass.depth = 0.0f;
		world_pass.stencil = 0;
		world_pass.render_area = { 0.0f, 0.0f, (f32)render_area_width, (f32)render_area_height };
		world_pass.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		world_pass.do_clear_color = true;
		world_pass.do_clear_depth = false;
		world_pass.do_clear_stencil = false;
		world_pass.load_last_pass = false;

		world_pass.pass = create_ref<WorldPass>();

		RendergraphGlobalDependency world_dependency;
		world_dependency.src_global_buffer = RendergraphGlobalDependency::GlobalColorBuffer;
		world_dependency.dest_resource_name = WORLD_COLOR_RESOURCE_NAME;

		world_pass.global_dependencies.push_back(world_dependency);

		return world_pass;
	}
}