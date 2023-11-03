#include "Renderer/Passes/WorldPass.h"

namespace hit
{
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
		return true;
	}

	void WorldPass::shutdown()
	{ 
	
	}

	void WorldPass::on_render(FrameData* frame_data)
	{ 
		
	}

	bool WorldPass::on_resize(ui32 new_width, ui32 new_height)
	{ 
		// images source is global, so swapchain will auto resize
		return true;
	}

	UnbakedPass world_pass_create_builtin_pass(ui32 render_area_width, ui32 render_area_height)
	{
		UnbakedPass world_pass;
		world_pass.depth = 0.0f;
		world_pass.stencil = 0;
		world_pass.render_area = { 0.0f, 0.0f, (f32)render_area_width, (f32)render_area_height };
		world_pass.clear_color = { 1.0f, 0.0f, 0.0f, 1.0f };
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