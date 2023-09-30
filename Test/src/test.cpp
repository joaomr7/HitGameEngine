 #include "TestFramework.h"
#include "Tests/MemoryTest.h"
#include "Tests/TypedArenaTest.h"
#include "Tests/FastTypedArenaTest.h"

using namespace hit;

#if defined(MSVC) && defined(HIT_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

int main()
{
#if defined(MSVC) && defined(HIT_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    TestSystem test_system;
    
    test_system.initialize();

    //add_memory_system_tests(test_system);
    add_typed_arena_tests(test_system);
    add_fast_typed_arena_tests(test_system);

    test_system.run_all();
    
    test_system.shutdown();

    return 0;
}