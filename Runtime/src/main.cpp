#include "HitEngineEntry.h"

namespace hit
{
    int hit_main(int argc, char** argv)
    {
        Engine engine;

        // initialize engine
        {
            EngineData data;

            data.game_name = "Runtime Hit Engine";
            data.main_window_width = 1280;
            data.main_window_height = 720;


            if(!engine.initialize(data))
            {
                hit_fatal("Failed to initialize engine!");
                return -1;
            }
        }

        //test script
        ExternalPackage* package = Platform::load_external_package("ScriptTest.dll");
        if(package)
        {
            int (*fun)(int) = reinterpret_cast<int(*)(int)>(Platform::get_package_function(package, "return_value"));
            Log::log_message(Log::LogLevel::Info, "Fun result: {}", fun(9));

            Platform::unload_external_package(package);
        }        

        engine.run();

        engine.shutdown();

        return 0;
    }
}