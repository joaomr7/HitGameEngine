#pragma once

#include "Core/Module.h"
#include "Utils/Ref.h"

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
        // backend api can access the frontend
        Ref<class RendererAPI> m_backend_renderer;
        friend class VulkanRenderer;
        friend class Engine;
    };
}