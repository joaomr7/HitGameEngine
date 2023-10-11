#include "Renderer/Renderer.h"

#include "Renderer/RendererAPI.h"
#include "Core/Engine.h"
#include "Core/Memory.h"

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

        return m_backend_renderer->initialize();
    }

    void Renderer::shutdown()
    {
        m_backend_renderer->shutdown();
    }

    bool Renderer::execute()
    {
        // begin scene
        if(!m_backend_renderer->begin_frame()) [[unlikely]]
        {
            // try to begin frame again, if fails, then return false
            if(!m_backend_renderer->begin_frame())
            {
                hit_error("Failed to begin renderer frame.");
                return false;
            }
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
}