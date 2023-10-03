namespace hit
{
    EngineData hit_main(int argc, char** argv);
}

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

    auto engine_data = hit::hit_main(argc, argv);

    hit::Engine engine;

    if(!engine.initialize(engine_data))
    {
        hit::Log::log_message(hit::Log::LogLevel::Fatal, "Failed to initialize engine!");
        return -1;
    }

    engine.run();

    engine.shutdown();

    return 0;
}