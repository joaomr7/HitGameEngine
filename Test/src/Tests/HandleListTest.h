#pragma once

#include "../TestFramework.h"
#include "Utils/HandleList.h"
#include "Utils/FastHandleList.h"

#include <vector>
#include <span>

namespace hit
{
    bool has_list_intersection(std::span<int> l1, std::span<int> l2)
    {
        for(auto i : l1)
        {
            for(auto j : l2)
            {
                if(i == j) return true;
            }
        }

        return false;
    }

    // HandleList test
    test_val handle_list_test_1()
    {
        const int list_size = 100000;
        const int remove_size = 10000;

        HandleList<int> list;

        std::vector<Handle<int>> handles;
        FastTypedArena<int> removed_values(remove_size);

        for(auto i = 0; i < list_size; i++)
        {
            handles.push_back(list.add(i));
            test_silent_check(handles.back().valid());
        }

        for(auto i = 0; i < remove_size; i++)
        {
            int random_number = test_random_number(0, handles.size());

            removed_values.push_back(*list.get(handles[random_number]));
            list.remove(handles[random_number]);

            handles.erase(handles.begin() + random_number);
        }

        test_check(!has_list_intersection(list.data(), removed_values.data()));
        test_check(list.size() + removed_values.size() == list_size);

        test_success();
    }

    // FastHandleList test
    test_val fast_handle_list_test_1()
    {
        const int list_size = 100000;
        const int remove_size = 10000;

        FastHandleList<int> list;

        std::vector<Handle<int>> handles;
        FastTypedArena<int> removed_values(remove_size);

        for(auto i = 0; i < list_size; i++)
        {
            handles.push_back(list.add(i));
            test_silent_check(handles.back().valid());
        }

        for(auto i = 0; i < remove_size; i++)
        {
            int random_number = test_random_number(0, handles.size());

            removed_values.push_back(*list.get(handles[random_number]));
            list.remove(handles[random_number]);

            handles.erase(handles.begin() + random_number);
        }

        test_check(!has_list_intersection(list.data(), removed_values.data()));
        test_check(list.size() + removed_values.size() == list_size);

        test_success();
    }

    void add_handle_list_tests(TestSystem& test_system)
    {
        test_system.add_test(get_test(handle_list_test_1));

        test_system.add_test(get_test(fast_handle_list_test_1));
    }
}