#include "HitEngineEntry.h"

namespace hit
{
    EngineData hit_main(int argc, char** argv)
    {
        EngineData data;

        data.game_name = "Runtime Hit Engine";
        data.main_window_width = 1280;
        data.main_window_height = 720;

        return data;
    }
}