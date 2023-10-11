#pragma once

namespace hit
{
    class Engine;
    class Renderer;

    // API to communicate between Renderer and Renderer'Backend'
    class RendererAPI
    {
    public:
        virtual ~RendererAPI() = default;

        virtual bool initialize() = 0;
        virtual void shutdown() = 0;

        virtual bool begin_frame() = 0;
        virtual bool end_frame() = 0;

    protected:
        const Engine* get_engine() { return m_engine; }
        const Renderer* get_frontend_renderer() { return m_frontend_renderer; }

    private:
        Engine* m_engine;
        Renderer* m_frontend_renderer;
        friend class Renderer;
    };
};