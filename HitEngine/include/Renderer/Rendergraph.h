#pragma once

#include "Core/Types.h"
#include "Renderer/Renderpass.h"
#include "Utils/Ref.h"

#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <unordered_map>

namespace hit
{
	class Renderer;
	class Rendergraph;
	class UnbakedRendergraph;

	struct FrameData { };

	struct RendergraphResource
	{
		enum Type
		{
			TypeColor,
			TypeDepth
		};

		enum Origin
		{
			OriginGlobal,	// global in rendergraph
			OrginExternal,	// from other pass -> get the external resource defining a dependency
			OriginSelf		// from current pass
		};

		std::string name;

		Type type;
		Origin origin;
		std::vector<Ref<Texture>> attachments;
	};

	class RendergraphPass
	{
	public:
		virtual ~RendergraphPass() = default;

		// it's called before initialize, so pass is not created yet
		virtual bool generate_resources(ui32 images_width, ui32 images_height) = 0;

		virtual bool initialize() = 0;
		virtual void shutdown() = 0;

		virtual void on_render(FrameData* frame_data) = 0;
		virtual bool on_resize(ui32 new_width, ui32 new_height) = 0;

		bool has_resource(const std::string& resource_name) const;

	protected:
		std::vector<RendergraphResource> m_resources;

		inline const Ref<Renderpass>& get_pass() const { return m_pass; }
		inline const Renderer* get_renderer() const { return m_renderer; }

	private:
		Ref<Renderpass> m_pass;
		Renderer* m_renderer;
		
		friend UnbakedRendergraph;
		friend Rendergraph;
	};

	struct RendergraphGlobalDependency
	{
		enum BufferType : ui8
		{
			GlobalColorBuffer,
			GlobalDepthBuffer,
			MaxGlobalBufferCount
		};

		BufferType src_global_buffer;
		std::string dest_resource_name;
	};

	struct RendergraphPassDependency
	{
		std::string dest_resource_name;
		std::string src_pass_name;
		std::string src_resource_name;
	};

	struct UnbakedPass
	{
		f32 depth;
		ui32 stencil;

		Vec4 render_area;
		Vec4 clear_color;

		bool do_clear_color;
		bool do_clear_depth;
		bool do_clear_stencil;

		bool load_last_pass;

		Ref<RendergraphPass> pass;
		std::vector<RendergraphGlobalDependency> global_dependencies;
		std::vector<RendergraphPassDependency> pass_dependencies;
	};

	class UnbakedRendergraph
	{
	public:
		UnbakedRendergraph() = default;
		~UnbakedRendergraph() = default;

		bool add_pass(const std::string& name, const UnbakedPass& pass);

	private:
		bool has_pass(const std::string& pass_name) const;
		bool has_pass_resource(const std::string& pass_name, const std::string& resource_name) const;

	private:
		std::vector<UnbakedPass> m_passes;
		std::unordered_map<std::string, ui32> m_passes_name_locator;

		friend Rendergraph;
	};

	class Rendergraph
	{
	public:
		Rendergraph() = default;
		~Rendergraph() = default;

		bool initialize(const Renderer* renderer, const UnbakedRendergraph& unbaked);
		void shutdown();

		bool on_render(FrameData* frame_data);
		bool on_resize(ui32 new_width, ui32 new_height);

		bool has_pass(const std::string& pass_name) const;
		const Ref<RendergraphPass> get_pass(const std::string& pass_name) const;

	private:
		std::array<RendergraphResource, RendergraphGlobalDependency::MaxGlobalBufferCount> m_global_resources;

		std::vector<Ref<RendergraphPass>> m_passes;
		std::unordered_map<std::string, ui32> m_passes_name_locator;
	};
}