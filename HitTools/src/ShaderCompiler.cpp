#include "Shader/ShaderCompiler.h"

#include "Core/Assert.h"
#include "File/File.h"
#include "File/SerialFile.h"
#include "Utils/Serializer.h"

#include <filesystem>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace hit::helper
{
    static shaderc_shader_kind shader_type_to_shader_c_type(ShaderProgram::Type type)
    {
        switch(type)
        {
            case ShaderProgram::Vertex: return shaderc_glsl_vertex_shader;
            case ShaderProgram::Fragment: return shaderc_glsl_fragment_shader;
        }

        hit_assert(false, "Invalid shader type");
    }
}

namespace hit
{
    ShaderProgram compile_shader_source(const ShaderSource& source)
    {
        ShaderProgram out_program = source.program;

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source.source, helper::shader_type_to_shader_c_type(source.program.type), "shader_source");
        if(module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            hit_assert(false, module.GetErrorMessage());
        }

        out_program.source = std::vector<ui32>(module.begin(), module.end());

        return out_program;
    }

    int shader_compiler_entry_point(const std::vector<std::string>& options)
    {
        if(options.size() < 2)
        {
            hit_error("Invalid shader_compile options!");
            hit_trace("shader_compile options: (output_directoy) (list of shader files to be compiled)");
            return -1;
        }

        const auto& output_directoy = options[0];
        const auto& files = std::vector(options.begin() + 1, options.end());

        for(auto& file : files)
        {
            File file_content;
            if(!file_read(file, File::Text, file_content))
            {
                hit_error("Failed to read file '{}'!", file);
                return false;
            }

            ShaderParser parser(file_content.read_as_text());

            if(!parser.has_configuration())
            {
                hit_error("Shader '{}', has no valid configuration!", file);
                continue;
            }

            std::vector<ShaderSource> shader_sources;

            if(parser.has_shader_type(ShaderProgram::Vertex))
            {
                shader_sources.push_back(parser.read_shader(ShaderProgram::Vertex));
            }

            if(parser.has_shader_type(ShaderProgram::Fragment))
            {
                shader_sources.push_back(parser.read_shader(ShaderProgram::Fragment));
            }

            std::vector<ShaderProgram> compiled_shaders;
            for(const auto& source : shader_sources)
            {
                compiled_shaders.push_back(compile_shader_source(source));
            }

            // serialize shader programs and save file
            std::string output_file_name = output_directoy + "/" + std::filesystem::path(file).stem().string() + ".hit_shader";
            SerialFileWriter out_writer(output_file_name);

            // serialize number of shaders
            out_writer.get_serializer().serialize(compiled_shaders.size());

            for(auto& program : compiled_shaders)
            {
                if(!program.serialize(out_writer.get_serializer()))
                {
                    hit_error("Failed to serialize shader program '{}'!", file);
                    break;
                }
            }

            if(!out_writer.save())
            {
                hit_error("Failed to save compiled shader file '{}'!", output_file_name);
                return -1;
            }
        }

        return 0;
    }
}