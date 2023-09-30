#pragma once

#include "../TestFramework.h"
#include "Core/Memory.h"

namespace hit
{
    // raw memory test
    test_val memory_allocation_deallocation_test()
    {
        auto memory = Memory::allocate_memory(32, Memory::Usage::Any);
        Memory::deallocate_memory(memory);

        test_success();
    }
    
    test_val memory_leak_test()
    {
        auto memory = Memory::allocate_memory(32, Memory::Usage::Any);

        //test_expect(Memory::shutdown_memory_system(), false);
        test_success();
    }

    test_val memory_realloc_test()
    {
        auto memory = Memory::allocate_memory(32, Memory::Usage::Any);
        memory = Memory::reallocate_memory(memory, 64);

        test_check(memory);

        Memory::deallocate_memory(memory);

        test_success();
    }

    test_val memory_copy_set_test()
    {
        auto m1 = (ui32*)Memory::allocate_memory(sizeof(ui32), Memory::Usage::Any);
        auto m2 = (ui32*)Memory::allocate_memory(sizeof(ui32), Memory::Usage::Any);

        Memory::set_memory((ui8*)m1, 1, sizeof(32));
        Memory::copy_memory((ui8*)m2, (ui8*)m1, sizeof(ui32));

        test_check(*m1 == *m2);

        Memory::deallocate_memory((ui8*)m1);
        Memory::deallocate_memory((ui8*)m2);

        test_success();
    }

    // memory usages tests
    test_val memory_allocation_deallocation_usage_test()
    {
        auto usage = Memory::allocate_usage(32, Memory::Usage::Any);
        Memory::deallocate_usage(usage);

        test_success();
    }

    test_val memory_leak_usage_test()
    {
        auto usage = Memory::allocate_usage(32, Memory::Usage::Any);

        //test_expect(Memory::shutdown_memory_system(), false);
        test_success();
    }

    test_val memory_realloc_usage_test()
    {
        auto usage = Memory::allocate_usage(32, Memory::Usage::Any);
        usage = Memory::reallocate_usage(usage, 64);

        test_check(usage.memory);
        test_check(usage.size == 64)

        Memory::deallocate_usage(usage);
        
        test_success();
    }

    test_val memory_copy_set_usage_test()
    {
        auto u1 = Memory::allocate_usage(sizeof(ui32), Memory::Usage::Any);
        auto u2 = Memory::allocate_usage(sizeof(ui32), Memory::Usage::Any);

        Memory::set_usage_memory(u1, 1, u1.size);
        Memory::copy_usage(u2, u1, u1.size);

        test_check(*(ui32*)u1.memory == *(ui32*)u2.memory);

        Memory::deallocate_usage(u1);
        Memory::deallocate_usage(u2);

        test_success();
    }

    void add_memory_system_tests(TestSystem& test_system)
    {
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
    }
}