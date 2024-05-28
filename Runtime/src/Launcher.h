#pragma once

#include "HitEngine.h"
#include "File/SerialFile.h"

namespace hit
{
	struct Vertex
	{
		Vec3 position;
		Vec2 uv;
	};

	class Launcher : public Module
	{
	protected:
		bool initialize() override
		{
			m_renderer = cast_ref<Renderer>(get_engine()->get_module("Renderer"));
			m_world_pass = m_renderer->get_pass("WorldPass");

			m_quad = m_renderer->acquire_buffer();
			m_std_shader = create_ref<StandardShader>(m_renderer.get());
			m_quad = m_renderer->acquire_buffer();
			m_quad_indices = m_renderer->acquire_buffer();

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

			if (!m_std_shader->create(m_world_pass->get_pass()))
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

				auto hit_texture = m_renderer->acquire_texture();
				if (!hit_texture->create(texture_info, data.data()))
				{
					hit_error("Failed to create texture.");
					return false;
				}

				hit_texture->set_release_mode(Releseable::ModeAtFirstOpportunity);
				m_std_material->set_base_texture(hit_texture);
			}

			f32 aspect_ratio = 16.f / 9.f;
			Mat4 projection = mat4_perspective(80.f, aspect_ratio, 0.0001f, 10000.f);

			m_global_data->set_projection_matrix(projection);
			m_global_data->set_view_matrix(mat4_identity());

			return true;
		}

		void shutdown() override
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
			m_renderer = nullptr;
		}

		bool execute() override
		{
			m_world_pass->add_render_command(
				ShaderCommand(m_std_shader, ShaderCommand::Action::Bind)
			);

			m_world_pass->add_render_command(
				ShaderWriteConstantCommand(m_std_shader, ShaderProgram::Vertex, sizeof(Mat4), &m_model)
			);

			m_world_pass->add_render_command(
				ShaderAttributeCommand(m_std_shader, m_global_data, ShaderAttributeCommand::Action::Bind)
			);

			m_world_pass->add_render_command(
				ShaderAttributeCommand(m_std_shader, m_std_material, ShaderAttributeCommand::Action::Bind)
			);

			m_world_pass->add_render_command(
				BufferDrawCommand(m_quad, 0, 4, true)
			);

			m_world_pass->add_render_command(
				BufferDrawCommand(m_quad_indices, 0, 6, false)
			);

			m_world_pass->add_render_command(
				ShaderAttributeCommand(m_std_shader, m_std_material, ShaderAttributeCommand::Action::Unbind)
			);

			m_world_pass->add_render_command(
				ShaderAttributeCommand(m_std_shader, m_global_data, ShaderAttributeCommand::Action::Unbind)
			);

			m_world_pass->add_render_command(
				ShaderCommand(m_std_shader, ShaderCommand::Action::Unbind)
			);

			return true;
		}

		void on_event(Event& event) override
		{
			EventHandler handler(event);
			handler.handle<WindowResizeEvent>([&](WindowResizeEvent& e)
			{
				f32 aspect_ratio = (f32)e.width / (f32)e.height;
				Mat4 projection = mat4_perspective(80.f, aspect_ratio, 0.0001f, 10000.f);

				m_global_data->set_projection_matrix(projection);

				return true;
			});
		}

	private:
		Ref<Renderer> m_renderer;
		Ref<RendergraphPass> m_world_pass;

		Mat4 m_model = mat4_identity();

		Ref<Buffer> m_quad;
		Ref<Buffer> m_quad_indices;
		Ref<Shader> m_std_shader;
		Ref<StandardGlobalData> m_global_data;
		Ref<StandardMaterial> m_std_material;
	};
}