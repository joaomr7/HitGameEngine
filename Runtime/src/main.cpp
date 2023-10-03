#include "HitEngineEntry.h"

namespace hit
{
    EngineData hit_main(int argc, char** argv)
    {
        EngineData data;

        data.game_name = "Runtime Test";

        return data;
    }
}