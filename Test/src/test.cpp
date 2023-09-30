 #include "TestFramework.h"
#include "Tests/MemoryTest.h"

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
    
    // memory tests
    {
        test_system.add_test(get_test(memory_allocation_deallocation_test));
        test_system.add_test(get_test(memory_leak_test));
        test_system.add_test(get_test(memory_realloc_test));
        test_system.add_test(get_test(memory_copy_set_test));
    }

    // memory test usages
    {
        test_system.add_test(get_test(memory_allocation_deallocation_usage_test));
        test_system.add_test(get_test(memory_leak_usage_test));
        test_system.add_test(get_test(memory_realloc_usage_test));
        test_system.add_test(get_test(memory_copy_set_usage_test));
    }

    test_system.run_all();
    
    test_system.shutdown();

    return 0;
}