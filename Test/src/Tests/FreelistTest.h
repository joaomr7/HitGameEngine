#pragma once

#include "../TestFramework.h"
#include "Utils/Freelist.h"


namespace hit
{
    test_val freelist_test_1()
    {
        FreelistCore core(10);

        ui64 out_offset;
        test_check(core.push_size(5, out_offset));

        test_check(out_offset == 0);

        test_check(core.push_size(5, out_offset));

        test_check(out_offset == 5);

        test_check(core.pop_size(5, out_offset));

        test_check(core.get_free_space() == 5);

        test_check(core.pop_size(5, 0));

        test_check(core.get_free_space() == 10);

        test_success();
    }

    test_val freelist_test_2()
    {
        FreelistCore core(20);

        ui64 out_offset;
        test_check(core.push_size(5, out_offset));

        test_check(out_offset == 0);

        test_check(core.get_free_space() == 15);

        test_check(core.push_size(10, out_offset));

        test_check(core.get_free_space() == 5);

        test_check(core.pop_size(5, out_offset));

        test_check(core.get_free_space() == 10);

        test_check(core.pop_size(5, 10));

        test_check(core.get_free_space() == 15);

        test_check(core.pop_size(5, 0));

        test_check(core.get_free_space() == 20);

        test_success();
    }

    test_val freelist_test_3()
    {
        FreelistCore core(10);

        ui64 out_offset;
        test_check(core.push_size(5, out_offset));

        test_check(out_offset == 0);

        test_check(core.resize(20));

        test_check(core.get_free_space() == 15);

        test_check(core.push_size(10, out_offset));

        test_check(core.get_free_space() == 5);

        test_check(core.pop_size(5, out_offset));

        test_check(core.get_free_space() == 10);

        test_check(core.pop_size(5, 10));

        test_check(core.resize(50));

        test_check(core.get_free_space() == 45);

        test_check(core.pop_size(5, 0));

        test_check(core.get_free_space() == 50);

        test_success();
    }

    void add_freelist_tests(TestSystem& test_system)
    {
        test_system.add_test(get_test(freelist_test_1));
        test_system.add_test(get_test(freelist_test_2));
        test_system.add_test(get_test(freelist_test_3));
    }
}