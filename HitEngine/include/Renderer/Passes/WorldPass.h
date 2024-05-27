#pragma once

#include "Renderer/Rendergraph.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shaders/StandardShader.h"

namespace hit
{
	constexpr const char* WORLD_COLOR_RESOURCE_NAME = "WorldColorBuffer";

	class WorldPass : public RendergraphPass
	{
	public:
		WorldPass() = default;
		~WorldPass() = default;

		bool generate_resources(ui32 images_width, ui32 images_height) override;

		bool initialize() override;
		void shutdown() override;

		void on_render(FrameData* frame_data) override;
		bool on_resize(ui32 new_width, ui32 new_height) override;

	private:
		// temporary
		Mat4 m_model = mat4_identity();

		Ref<Buffer> m_quad;
		Ref<Buffer> m_quad_indices;
		Ref<Shader> m_std_shader;
		Ref<StandardGlobalData> m_global_data;
		Ref<StandardMaterial> m_std_material;
	};

	// used for default world pass creation
	UnbakedPass world_pass_create_builtin_pass(ui32 render_area_width, ui32 render_area_height);
}