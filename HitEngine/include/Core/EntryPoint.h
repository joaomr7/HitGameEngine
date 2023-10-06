namespace hit
{
    int hit_main(int argc, char** argv);
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
    return hit::hit_main(argc, argv);
}