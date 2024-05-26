#pragma once

#include "Core/Module.h"
#include "Utils/Ref.h"

#include "Texture.h"
#include "RenderPipeline.h"
#include "Renderpass.h"
#include "Rendergraph.h"
#include "Buffer.h"

#include <string>
#include <vector>

namespace hit
{
    enum class RendererBackend
    {
        Vulkan
    };

    struct RendererConfiguration
    {
        RendererBackend backend;

        bool vsync;
        bool power_save_mode;
    };

    class Renderer : public Module
    {
    public:
        inline ui16 get_frame_width() const { return m_frame_width; }
        inline ui16 get_frame_height() const { return m_frame_height; }

        Ref<Renderpass> acquire_renderpass() const;
        Ref<RenderPipeline> acquire_render_pipeline() const;
        Ref<Buffer> acquire_buffer() const;

        std::vector<Ref<Texture>> get_swapchain_images() const;

    public:
        bool has_pass(const std::string& name) const;
        Ref<Renderpass> get_pass(const std::string& name) const;

    protected:
        bool initialize() override;
        void shutdown() override;
        bool execute() override;

    private:
        ui16 m_frame_width;
        ui16 m_frame_height;
        ui64 m_frame_generation;
        ui64 m_frame_last_generation;

    private:
        void resize(ui16 width, ui16 height);

    private:
        FrameData m_frame_data;
        Rendergraph m_graph;

    private:
        // backend api can access the frontend
        Ref<class RendererAPI> m_backend_renderer;
        friend class VulkanRenderer;
        friend class Engine;
    };
}