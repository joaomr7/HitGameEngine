#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"

#include "ShaderTypes.h"
#include "RenderPipeline.h"
#include "Renderer.h"

#include <string>
#include <string_view>
#include <vector>

namespace hit
{
    class Shader;

    class ShaderAttribute
    {
    public:
        virtual ~ShaderAttribute() = default;

        virtual bool create_resources() = 0;
        virtual void release_resources() = 0;

        inline const Shader* get_shader() const { return m_shader; }
        inline const PipelineInstance get_instance() const { return m_instance; }

    private:
        Shader* m_shader;
        PipelineInstance m_instance;
        friend class Shader;
    };

    class Shader
    {
    public:
        Shader(Renderer* renderer) : m_renderer(renderer) { }
        virtual ~Shader() = default;

        virtual bool create(const Ref<Renderpass>& pass) = 0;
        virtual void destroy() = 0;

        bool bind();
        bool unbind();

        virtual Ref<ShaderAttribute> create_program_attribute(ShaderProgram::Type program_type) = 0;
        virtual void destroy_program_attribute(const Ref<ShaderAttribute>& attribute) = 0;

        bool bind_attribure(const Ref<ShaderAttribute>& attribute);
        bool unbind_attribure(const Ref<ShaderAttribute>& attribute);

        bool has_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name);
        ui64 get_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name);

        bool write_data(ShaderAttribute* attribute, ui64 offset, ui64 size, void* data);
        bool write_constant(ShaderProgram::Type at, ui64 size, void* data);

    protected:
        bool load_programs(std::string_view programs_path, std::vector<ShaderProgram>& programs);

        bool register_attribute(ShaderProgram::Type program_type, const Ref<ShaderAttribute>& attribute);
        void unregister_attribute(const Ref<ShaderAttribute>& attribute);

    protected:
        bool create_pipeline(const PipelineConfig& config);
        void destroy_pipeline();

        RenderPipeline* get_pipeline() { return m_pipeline.get(); }

    private:
        Renderer* m_renderer;
        Ref<RenderPipeline> m_pipeline;
    };
}