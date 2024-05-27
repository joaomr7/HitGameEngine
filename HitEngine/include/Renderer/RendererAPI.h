#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"

#include <vector>

namespace hit
{
    class Engine;
    class Renderer;
    class Renderpass;
    class RenderPipeline;
    class Texture;
    class Buffer;

    // API to communicate between Renderer and Renderer'Backend'
    class RendererAPI
    {
    public:
        virtual ~RendererAPI() = default;

        virtual ui32 get_swapchain_image_count() const = 0;
        virtual const Ref<Texture> get_swapchain_image(ui32 index) const = 0;
        virtual std::vector<Ref<Texture>> get_swapchain_images() const = 0;

        virtual Ref<Texture> acquire_texture() = 0;
        virtual Ref<Renderpass> acquire_renderpass() = 0;
        virtual Ref<RenderPipeline> acquire_render_pipeline() = 0;
        virtual Ref<Buffer> acquire_buffer() = 0;

    protected:
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;

        virtual bool begin_frame() = 0;
        virtual bool end_frame() = 0;

        const Engine* get_engine() { return m_engine; }
        const Renderer* get_frontend_renderer() { return m_frontend_renderer; }

    private:
        Engine* m_engine;
        Renderer* m_frontend_renderer;
        friend class Renderer;
    };
};