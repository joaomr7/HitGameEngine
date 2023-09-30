#pragma once

#include "../TestFramework.h"
#include "Utils/FastTypedArena.h"

#include <string.h>

namespace hit
{
    test_val fast_typed_arena_test_1()
    {
        FastTypedArena<int> t_arena;
        test_check(t_arena.create(10));

        test_check(t_arena.capacity() == 10);
        test_check(t_arena.size() == 0);

        for(auto i = 0; i < 10; i++)
        {
            test_check(t_arena.push_back(i));
        }

        test_check(t_arena.capacity() == 10);
        test_check(t_arena.size() == 10);

        for(auto i = 0; i < 10; i++)
        {
            t_arena.pop_back();
        }

        test_check(t_arena.capacity() == 10);
        test_check(t_arena.size() == 0);

        test_success();
    }

    test_val fast_typed_arena_test_2()
    {
        FastTypedArena<int> t_arena;
        t_arena.create(10);
        for(auto i = 0; i < 10; i++)
            test_check(t_arena.emplace_back(i));

        FastTypedArena<int> t_copy = t_arena;
        t_copy = t_arena;

        int* arena_memory = t_arena.data().data();
        int* copy_memory = t_arena.data().data();

        test_check(t_copy.capacity() == t_arena.capacity());
        test_check(t_copy.size() == t_arena.size());
        test_check(std::memcmp(arena_memory, copy_memory, 10 * sizeof(int)) == 0);

        test_success();
    }

    test_val fast_typed_arena_test_3()
    {
        FastTypedArena<int> t_arena;
        t_arena.create(10);
        for(auto i = 0; i < 10; i++)
            test_check(t_arena.push_array(i + 2));

        test_check(t_arena.capacity() > 10);
        test_check(t_arena.size() > 10);

        t_arena.pop_array_back(10);

        test_check(t_arena.capacity() > 10);
        test_check(t_arena.size() > 10);

        t_arena.clear();

        test_check(t_arena.empty());
        test_check(t_arena.capacity() > 10);

        test_success();
    }

    test_val fast_typed_arena_test_4()
    {
        FastTypedArena<int> t_arena;
        t_arena.create(10);
        for(auto i = 0; i < 10; i++)
            test_check(t_arena.push_back(i + 2));

        for(auto i = 0; i < 10; i++)
        {
            hit_trace("{}", *t_arena[i]);
        }

        test_success();
    }

    test_val fast_typed_arena_test_5()
    {
        FastTypedArena<int> t_arena_1;
        t_arena_1.create(10);
        for(auto i = 0; i < 10; i++)
            test_check(t_arena_1.push_back(i + 2));

        FastTypedArena<int> t_arena_2 = std::move(t_arena_1);

        for(auto i = 0; i < 10; i++)
        {
            hit_trace("{}", *t_arena_2[i]);
        }

        test_success();
    }

    void add_fast_typed_arena_tests(TestSystem& test_system)
    {
        test_system.add_test(get_test(fast_typed_arena_test_1));
        test_system.add_test(get_test(fast_typed_arena_test_2));
        test_system.add_test(get_test(fast_typed_arena_test_3));
        test_system.add_test(get_test(fast_typed_arena_test_4));
        test_system.add_test(get_test(fast_typed_arena_test_5));
    }
}