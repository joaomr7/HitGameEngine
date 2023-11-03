#pragma once

#include "Renderer/Rendergraph.h"

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
	};

	// used for default world pass creation
	UnbakedPass world_pass_create_builtin_pass(ui32 render_area_width, ui32 render_area_height);
}