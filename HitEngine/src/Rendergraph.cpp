#include "Renderer/Rendergraph.h"
#include "Renderer/Renderer.h"

#include <ranges>
#include <set>

namespace hit::helper
{
	Attachment::Type resource_type_to_attachment_type(RendergraphResource::Type type)
	{
		switch(type)
		{
			case hit::RendergraphResource::TypeColor:	return Attachment::TypeColor;
			case hit::RendergraphResource::TypeDepth:	return Attachment::TypeDepth;
		}
	}
}

namespace hit
{
	bool hit::RendergraphPass::has_resource(const std::string& resource_name) const
	{
		for(auto& resource : m_resources)
		{
			if(resource.name == resource_name)
			{
				return true;
			}
		}

		return false;
	}

	bool Rendergraph::initialize(const Renderer* renderer, const UnbakedRendergraph& unbaked)
	{
		if(!renderer || unbaked.m_passes.empty())
		{
			hit_error_if(!renderer, "Can't initialize rendergraph with a null renderer ptr!");
			hit_error_if(unbaked.m_passes.empty(), "Can't initialize rendergraph with no pass!");
			return false;
		}

		// create global resources
		{
			RendergraphResource global_color_buffer;
			global_color_buffer.name = "GlobalColorBuffer";
			global_color_buffer.type = RendergraphResource::TypeColor;
			global_color_buffer.origin = RendergraphResource::OriginSelf;
			global_color_buffer.attachments = renderer->get_swapchain_images();

			m_global_resources[RendergraphGlobalDependency::GlobalColorBuffer] = global_color_buffer;
		}

		// get image size
		const ui32 image_width = renderer->get_frame_width();
		const ui32 image_height = renderer->get_frame_height();

		// generate all pass resources
		for(auto& unbaked : unbaked.m_passes)
		{
			unbaked.pass->m_renderer = (Renderer*)renderer;

			if(!unbaked.pass->generate_resources(image_width, image_height))
			{
				hit_error("Failed to generate rendergraph pass resources!");
				return false;
			}

			if(unbaked.pass->m_resources.empty())
			{
				hit_error("Pass resources are empty!");
				return false;
			}
		}

		// fill dependencies and check if swapchain image is used
		bool has_pass_linked_to_color_global_buffer = false;
		for(auto& unbaked_pass : unbaked.m_passes)
		{
			auto& pass = unbaked_pass.pass;

			// global dependencies
			if(!unbaked_pass.global_dependencies.empty())
			{
				for(auto& [global_src, dest_resource_name] : unbaked_pass.global_dependencies)
				{
					// check if destination exists
					if(!pass->has_resource(dest_resource_name))
					{
						hit_error("Pass resource '{}', do not exists!", dest_resource_name);
						return false;
					}

					// get destination resource
					RendergraphResource* dest_resource;
					for(auto& resource : pass->m_resources)
					{
						if(resource.name == dest_resource_name)
						{
							dest_resource = &resource;
							break;
						}
					}

					// check if dependency match
					const auto& global_resource = m_global_resources[global_src];
					if(dest_resource->origin != RendergraphResource::OriginGlobal)
					{
						hit_error("Invalid global dependency in '{}'!", dest_resource_name);
						return false;
					}

					if(dest_resource->type != global_resource.type)
					{
						hit_error("Invalid global dependency type in '{}'", dest_resource_name);
						return false;
					}

					dest_resource->attachments = global_resource.attachments;

					if(global_resource.type == RendergraphResource::TypeColor)
					{
						has_pass_linked_to_color_global_buffer = true;
					}
				}
			}
		
			// pass dependencies
			if(!unbaked_pass.pass_dependencies.empty())
			{
				for(auto& [dest_name, source_pass, resource_name] : unbaked_pass.pass_dependencies)
				{
					// check if destination exists
					if(!pass->has_resource(dest_name))
					{
						hit_error("Pass resource '{}', do not exists!", dest_name);
						return false;
					}

					// check if dependency exists
					if(!unbaked.has_pass_resource(source_pass, resource_name))
					{
						hit_error("Pass '{}' resource '{}', do not exists!", source_pass, resource_name);
						return false;
					}

					// get destination resource
					RendergraphResource* dest_resource;
					for(auto& resource : pass->m_resources)
					{
						if(resource.name == dest_name)
						{
							dest_resource = &resource;
							break;
						}
					}

					// get source resource
					auto& src_pass = unbaked.m_passes[m_passes_name_locator.at(source_pass)].pass;
					RendergraphResource* src_resource;
					for(auto& resource : src_pass->m_resources)
					{
						if(resource.name == resource_name)
						{
							src_resource = &resource;
							break;
						}
					}

					// check if dependency match
					if(dest_resource->origin != RendergraphResource::OrginExternal)
					{
						hit_error("Invalid pass dependency: '{}' -> '{}'", source_pass, dest_name);
						return false;
					}

					if(dest_resource->type != src_resource->type)
					{
						hit_error("Invalid pass dependency type: '{}' -> '{}'", resource_name, dest_name);
						return false;
					}

					dest_resource->attachments = src_resource->attachments;
				}
			}
		}

		if(!has_pass_linked_to_color_global_buffer)
		{
			hit_error("No pass linked to global color resource!");
			return false;
		}

		// find present pass which is the last pass linked to global color buffer
		Ref<RendergraphPass> present_pass = nullptr;
		for(auto& pass : unbaked.m_passes | std::ranges::views::reverse)
		{
			for(auto& resource : pass.pass->m_resources)
			{
				if(resource.origin == RendergraphResource::OriginGlobal && resource.type == RendergraphResource::TypeColor)
				{
					present_pass = pass.pass;
					break;
				}
			}

			if(present_pass) break;
		}

		if(!present_pass)
		{
			hit_error("No present pass found!");
			return false;
		}

		// create renderpasses
		for(auto& unbaked_pass : unbaked.m_passes)
		{
			RenderpassConfig pass_config;
			pass_config.depth = unbaked_pass.depth;
			pass_config.stencil = unbaked_pass.stencil;
			pass_config.render_area = unbaked_pass.render_area;
			pass_config.clear_color = unbaked_pass.clear_color;

			pass_config.clear_flag = 0;
			if(unbaked_pass.do_clear_color)   pass_config.clear_flag |= RenderpassConfig::ClearColor;
			if(unbaked_pass.do_clear_depth)   pass_config.clear_flag |= RenderpassConfig::ClearDepth;
			if(unbaked_pass.do_clear_stencil) pass_config.clear_flag |= RenderpassConfig::ClearStencil;

			pass_config.attachment_width = image_width;
			pass_config.attachemnt_height = image_height;

			pass_config.present_after = unbaked_pass.pass == present_pass;

			// generate attachments
			for(const auto&  resource : unbaked_pass.pass->m_resources)
			{
				Attachment attachment;
				attachment.type = helper::resource_type_to_attachment_type(resource.type);
				
				if(pass_config.clear_flag > 0)
				{
					attachment.load_op = Attachment::OpClear;
				}
				else if(unbaked_pass.load_last_pass)
				{
					attachment.load_op = Attachment::OpLoad;
				}
				else
				{
					attachment.load_op = Attachment::OpLoadDontCare;
				}

				attachment.store_op = Attachment::OpStore;

				attachment.format = resource.attachments[0]->get_format();
				attachment.attachments = resource.attachments;

				pass_config.attachments.push_back(attachment);
			}

			auto renderpass = renderer->acquire_renderpass();
			if(!renderpass->create(pass_config))
			{
				hit_error("Failed to create renderpass!");
				return false;
			}

			unbaked_pass.pass->m_pass = renderpass;

			// add baked pass to graph
			m_passes.push_back(unbaked_pass.pass);
		}

		// copy pass locator
		m_passes_name_locator = unbaked.m_passes_name_locator;

		// initialize graph passes
		for(auto& pass : m_passes)
		{
			if(!pass->initialize())
			{
				hit_error("Failed to initialize renderpass!");
				return false;
			}
		}

		return true;
	}

	void Rendergraph::shutdown()
	{ 
		if(!m_passes.empty())
		{
			for(auto& pass : m_passes)
			{
				pass->shutdown();
				pass->m_pass->destroy();
				pass->m_pass = nullptr;
				pass = nullptr;
			}

			m_passes.clear();
		}
	}

	bool Rendergraph::on_render(FrameData* frame_data)
	{ 
		for(auto& pass : m_passes)
		{
			auto& renderpass = pass->m_pass;

			if(!renderpass->begin()) [[unlikely]]
			{
				hit_error("Failed to begin renderpass!");
				return false;
			}

			pass->on_render(frame_data);

			renderpass->end();
		}

		return true;
	}

	bool Rendergraph::on_resize(ui32 new_width, ui32 new_height)
	{
		// resize passes
		for(auto& pass : m_passes)
		{
			if(!pass->on_resize(new_width, new_height))
			{
				hit_error("Failed to resize rendergraph pass!");
				return false;
			}
		}

		// regenerate framebuffers
		for(auto& pass : m_passes)
		{
			if(!pass->m_pass->resize(new_width, new_height))
			{
				hit_error("Failed to regenerate framebuffers from renderpass!");
				return false;
			}
		}

		return true;
	}

	bool Rendergraph::has_pass(const std::string& pass_name) const
	{
		return m_passes_name_locator.find(pass_name) != m_passes_name_locator.end();
	}

	const Ref<RendergraphPass> Rendergraph::get_pass(const std::string& pass_name) const
	{
		return m_passes[m_passes_name_locator.at(pass_name)];
	}

	// Unbaked Graph
	bool UnbakedRendergraph::add_pass(const std::string& name, const UnbakedPass& pass)
	{
		// check if resources names are unique
		if(has_pass(name))
		{
			hit_error("Already exists a pass called '{}'!", name);
			return false;
		}

		std::vector<std::string> resources_names_list;
		for(auto& resource : pass.pass->m_resources)
		{
			resources_names_list.push_back(resource.name);
		}

		std::set<std::string> unique_names_set(resources_names_list.begin(), resources_names_list.end());
		if(unique_names_set.size() != resources_names_list.size())
		{
			hit_error("Pass resources names are not unique!");
			return false;
		}

		m_passes.push_back(pass);
		m_passes_name_locator[name] = m_passes.size() - 1;

		return true;
	}

	bool UnbakedRendergraph::has_pass(const std::string& pass_name) const
	{
		return m_passes_name_locator.find(pass_name) != m_passes_name_locator.end();
	}

	bool UnbakedRendergraph::has_pass_resource(const std::string& pass_name, const std::string& resource_name) const
	{
		auto pass_index_it = m_passes_name_locator.find(pass_name);

		if(pass_index_it == m_passes_name_locator.end())
		{
			return false;
		}

		auto& pass = m_passes[pass_index_it->second].pass;
		
		for(auto& resource : pass->m_resources)
		{
			if(resource.name == resource_name)
			{
				return true;
			}
		}

		return false;
	}
}