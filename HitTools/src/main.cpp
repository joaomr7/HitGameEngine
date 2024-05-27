#include "Shader/ShaderCompiler.h"
#include "Image/ImageLoader.h"

#include "Core/Memory.h"
#include "Core/Log.h"

using namespace hit;

#if defined(MSVC) && defined(HIT_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

int main(int argc, char** argv)
{
#if defined(MSVC) && defined(HIT_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // initializing core systems
    {
        if(!Log::initialize_log_system())
        {
            return -1;
        }

        if(!Memory::initialize_memory_system())
        {
            hit_error("Failed to initialize memory system!");
            return -1;
        }
    }

    // get args
    std::vector<std::string> args;
    args.resize(argc - 1);

    for(int i = 1; auto& arg : args)
    {
        arg = argv[i++];
    }

    if(args.empty())
    {
        hit_info("Usage: HitTools -[module] [module options]");
        hit_trace("Available modules:");
        hit_trace("\t 1. shader_compile [options: (output_directoy) (list of shader files to be compiled)]");
        hit_trace("\t 2. image_loader [options: (output_directoy) (list of images files to load)]");
        return 0;
    }

    int return_value = 0;

    if(argc > 0)
    {
        auto module = args[0];
        auto options = std::vector<std::string>(args.begin() + 1, args.end());

        if(module == "-shader_compile")
        {
            return_value = shader_compiler_entry_point(options);
        }
        else if (module == "-image_loader")
        {
            return_value = image_loader_entry_point(options);
        }
        else
        {
            hit_error("Invalid module '{}'!", module);
            return_value = -1;
        }
    }

    // shutting down core systems
    {
        if(!Memory::shutdown_memory_system())
        {
            hit_error("Memory leak detected!");
        }

        Log::shutdown_log_system();
    }

    return return_value;
}