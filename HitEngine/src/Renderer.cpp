#include "Renderer/Renderer.h"

#include "Renderer/RendererAPI.h"
#include "Core/Engine.h"
#include "Core/Memory.h"

#include "Renderer/Passes/WorldPass.h"

// Vulkan API
#include "VulkanRenderer.h"

namespace hit
{
    bool Renderer::initialize()
    {
        const RendererConfiguration& configuration = get_engine()->get_renderer_config();

        switch(configuration.backend)
        {
            case RendererBackend::Vulkan:
            {
                m_backend_renderer = create_ref<VulkanRenderer>();
                break;
            };
            
            default: hit_assert(false, "Invalid Renderer backend");
        }

        // setup backend required variables
        m_backend_renderer->m_engine = (Engine*)get_engine();
        m_backend_renderer->m_frontend_renderer = this;

        m_frame_generation = 0;
        m_frame_last_generation = 0;
        m_frame_width = get_engine()->get_window_width();
        m_frame_height = get_engine()->get_window_height();

        if(!m_backend_renderer->initialize())
        {
            hit_error("Failed to initialize renderer!");
            return false;
        }

        // TODO: make it configurable outside renderer
        UnbakedRendergraph unbaked_graph;
        UnbakedPass world_pass = world_pass_create_builtin_pass(m_frame_width, m_frame_height);

        if(!unbaked_graph.add_pass("WorldPass", world_pass))
        {
            hit_error("Failed to add WorldPass!");
            return false;
        }

        if(!m_graph.initialize(this, unbaked_graph))
        {
            hit_error("Failed to initialize rendergraph!");
            return false;
        }

        return true;
    }

    void Renderer::shutdown()
    {
        m_graph.shutdown();
        m_backend_renderer->shutdown();
    }

    bool Renderer::execute()
    {
        if(m_frame_width == 0 || m_frame_height == 0) [[unlikely]]
        {
            return false;
        }

        bool update_graph = m_frame_generation != m_frame_last_generation;

        // begin scene
        if(!m_backend_renderer->begin_frame()) [[unlikely]]
        {
            if(!m_backend_renderer->begin_frame())
            {
                hit_error("Failed to begin renderer frame.");
                return false;
            }
        }

        if(update_graph) [[unlikely]]
        {
            if(!m_graph.on_resize(m_frame_width, m_frame_height))
            {
                hit_error("Failed to resize render graph!");
                return false;
            }
        }

        if(!m_graph.on_render()) [[unlikely]]
        {
            hit_error("Render graph failed!");
            return false;
        }

        // end scene
        if(!m_backend_renderer->end_frame()) [[unlikely]]
        {
            hit_error("Failed to end renderer frame.");
            return false;
        }

        return true;
    }

    void Renderer::resize(ui16 width, ui16 height)
    {
        m_frame_width = width;
        m_frame_height = height;
        m_frame_generation++;
    }

    Ref<Texture> Renderer::acquire_texture() const
    {
        return m_backend_renderer->acquire_texture();
    }

    Ref<Renderpass> Renderer::acquire_renderpass() const
    {
        return m_backend_renderer->acquire_renderpass();
    }

    Ref<RenderPipeline> Renderer::acquire_render_pipeline() const
    {
        return m_backend_renderer->acquire_render_pipeline();
    }

    Ref<Buffer> Renderer::acquire_buffer() const
    {
        return m_backend_renderer->acquire_buffer();
    }

    std::vector<Ref<Texture>> Renderer::get_swapchain_images() const
    {
        return m_backend_renderer->get_swapchain_images();
    }

    bool Renderer::has_pass(const std::string& name) const
    {
        return m_graph.has_pass(name);
    }

    Ref<RendergraphPass> Renderer::get_pass(const std::string& name) const
    {
        return m_graph.get_pass(name);
    }
}